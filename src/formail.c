/************************************************************************
 *	formail - The mail (re)formatter				*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: formail.c,v 1.21 1993/02/02 15:27:07 berg Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 1993/02/02 15:27:07 $";
#include "includes.h"
#include <ctype.h>		/* iscntrl() */
#include "formail.h"
#include "sublib.h"
#include "shell.h"
#include "common.h"
#include "fields.h"
#include "ecommon.h"
#include "formisc.h"

static const char unknown[]=UNKNOWN,re[]=" Re:",fmusage[]=FM_USAGE,
 From_[]=		FROM,				/* VNIX 'From ' line */
 Article_[]=		"Article ",		   /* USENET 'Article ' line */
 x_[]=			"X-",				/* general extension */
 old_[]=		OLD_PREFIX;			     /* my extension */
#define ssl(str)		str,STRLEN(str)
#define bsl(str)		{ssl(str)}
#define sslbar(str,bar1,bar2)	{ssl(str),STRLEN(bar1)-1,STRLEN(bar2)-1}
#include "header.h"
/*
 *	sender determination fields in order of importance/reliability
 *	reply-address determination fields (wrepl specifies the weight for
 *	for regular replies, wtrepl specifies the weight for trusted users)
 *
 *	I bet this is the first time you see a bar graph in C-source-code :-)
 */
static const struct {const char*head;int len,wrepl,wtrepl;}sest[]=
{ sslbar(replyto	,"******"	,"********"	),
  sslbar(Fromm		,"*"		,"****"		),
  sslbar(retreceiptto	,"********"	,"*******"	),
  sslbar(sender		,"*****"	,"******"	),
  sslbar(res_replyto	,"***********"	,"***********"	),
  sslbar(res_from	,"***foo***"	,"***bar***"	),
  sslbar(res_sender	,"**********"	,"**********"	),
  sslbar(errorsto	,"*******"	,"*****"	),
  sslbar(path		,"**"		,"*"		),
  sslbar(returnpath	,"***"		,"***"		),
  sslbar(From_		,"****"		,"**"		)
};

static struct saved rex[]=
{ bsl(subject),bsl(references),bsl(messageid),bsl(date)
};
#define subj	(rex+0)
#define refr	(rex+1)
#define msid	(rex+2)
#define hdate	(rex+3)

#ifdef sMAILBOX_SEPARATOR
#define emboxsep	smboxsep
#define MAILBOX_SEPARATOR
static const char smboxsep[]=sMAILBOX_SEPARATOR;
#endif /* sMAILBOX_SEPARATOR */
#ifdef eMAILBOX_SEPARATOR
#ifdef emboxsep
#undef emboxsep
#else
#define MAILBOX_SEPARATOR
#endif
static const char emboxsep[]=eMAILBOX_SEPARATOR;
#endif /* eMAILBOX_SEPARATOR */

const char binsh[]=BinSh,sfolder[]=FOLDER,
 couldntw[]="Couldn't write to stdout";
int errout,oldstdout,quiet,buflast;
pid_t child= -1;
FILE*mystdout;
size_t nrskip,nrtotal= -1,buflen,buffilled;
long totallen;
char*buf,*logsummary;
struct field*rdheader;
static struct field*iheader,*Iheader,*aheader,*Aheader,*xheader,*Xheader,
 *Rheader,*nheader;

static void logfolder P((void))	 /* estimate the no. of characters needed to */
{ size_t i;char num[8*sizeof totallen*4/10+1];	       /* represent totallen */
  static const char tabchar[]=TABCHAR;
  if(logsummary)
   { putssn(sfolder,STRLEN(sfolder));i=strlen(logsummary)+STRLEN(sfolder);
     i-=i%TABWIDTH;
     do putssn(tabchar,STRLEN(tabchar));
     while((i+=TABWIDTH)<LENoffset);
     ultstr(7,totallen,num);putssn(num,strlen(num));putcs('\n');
   }
}
    /* checks if the last field in rdheader looks like a known digest header */
static digheadr P((void))
{ char*chp;int i;size_t j;struct field*fp;
  for(fp=rdheader;fp->fld_next;fp=fp->fld_next);	 /* skip to the last */
  i=maxindex(cdigest);chp=fp->fld_text;j=fp->id_len;
  while((cdigest[i].lnr!=j||strnIcmp(cdigest[i].hedr,chp,j))&&i--);
  return i>=0||j>STRLEN(old_)&&!strnIcmp(old_,chp,STRLEN(old_))||
   j>STRLEN(x_)&&!strnIcmp(x_,chp,STRLEN(x_));
}

static artheadr P((void))	     /* could it be the start of an article? */
{ if(!rdheader&&!strncmp(buf,Article_,STRLEN(Article_)))
   { addbuf();rdheader->id_len=STRLEN(Article_);return 1;
   }
  return 0;
}

static PROGID;

main(lastm,argv)const char*const argv[];
{ int i,split=0,force=0,bogus=1,every=0,areply=0,trust=0,digest=0,nowait=0,
   keepb=0,minfields=(char*)progid-(char*)progid,conctenate=0;
  size_t j,lnl;char*chp,*namep;struct field*fldp,*fp2,**afldp,*fdate;
  if(lastm)			       /* sanity check, any argument at all? */
#define Qnext_arg()	if(!*chp&&!(chp=(char*)*++argv))goto usg
     while(chp=(char*)*++argv)
      { if((lastm= *chp++)==FM_SKIP)
	   goto number;
	else if(lastm!=FM_TOTAL)
	   goto usg;
	for(;;)
	 { switch(lastm= *chp++)
	    { case FM_TRUST:trust=1;continue;
	      case FM_REPLY:areply=1;continue;
	      case FM_FORCE:force=1;continue;
	      case FM_EVERY:every=1;continue;
	      case FM_DIGEST:digest=1;continue;
	      case FM_NOWAIT:nowait=1;continue;
	      case FM_KEEPB:keepb=1;continue;
	      case FM_CONCATENATE:conctenate=1;continue;
	      case FM_QUIET:quiet=1;continue;
	      case FM_LOGSUMMARY:Qnext_arg();
		 if(strlen(logsummary=chp)>MAXfoldlen)
		    chp[MAXfoldlen]='\0';
		 detab(chp);break;
	      case FM_SPLIT:split=1;
		 if(!*chp&&*++argv)
		    goto parsedoptions;
		 goto usg;
	      case HELPOPT1:case HELPOPT2:elog(fmusage);elog(FM_HELP);
		 goto xusg;
	      case FM_MINFIELDS:Qnext_arg();chp++;
	      default:chp--;
number:		 if(*chp-'0'>(unsigned)9)	    /* the number is not >=0 */
		    goto usg;
		 i=strtol(chp,&chp,10);
		 switch(lastm)			/* where does the number go? */
		  { case FM_SKIP:nrskip=i;break;
		    case FM_MINFIELDS:minfields=i;break;
		    default:nrtotal=i;
		  }
		 continue;
	      case FM_BOGUS:bogus=0;continue;
	      case FM_ADD_IFNOT:case FM_ADD_ALWAYS:case FM_REN_INSERT:
	      case FM_DEL_INSERT:case FM_EXTRACT:case FM_EXTRC_KEEP:
	      case FM_ReNAME:Qnext_arg();
		 if(!breakfield(chp,lnl=strlen(chp)))
		    goto invfield;
		 chp[lnl]='\n';			       /* terminate the line */
		 afldp=addfield(lastm==FM_REN_INSERT?&iheader:
		  lastm==FM_DEL_INSERT?&Iheader:lastm==FM_ADD_IFNOT?&aheader:
		  lastm==FM_ADD_ALWAYS?&Aheader:lastm==FM_EXTRACT?&xheader:
		  lastm==FM_EXTRC_KEEP?&Xheader:&Rheader,chp,++lnl);
		 if(lastm==FM_ReNAME)	      /* then we need a second field */
		  { int copied=0;
		    for(namep=(chp=(fldp= *afldp)->fld_text)+lnl,
		     chp+=lnl=fldp->id_len;chp<namep;++chp)
		     { switch(*chp)			  /* skip whitespace */
			{ case ' ':case '\t':case '\n':continue;
			}
		       break;
		     }				   /* second field attached? */
		    if(i=breakfield(chp,(size_t)(namep-chp)))  /* squeeze on */
		       tmemmove(fldp->fld_text+lnl,chp,i),copied=1;
		    else if(!(chp=(char*)*++argv)||	 /* look at next arg */
		     !(i=breakfield(chp,strlen(chp))))		/* no field? */
invfield:	     { nlog("Invalid field-name:");logqnl(chp?chp:"");
		       goto usg;
		     }
		    *afldp=fldp=
		     realloc(fldp,FLD_HEADSIZ+(fldp->tot_len=lnl+i));
		    if(!copied)			   /* if not squeezed on yet */
		       tmemmove(fldp->fld_text+lnl,chp,i);    /* squeeze now */
		  }
	      case '\0':;
	    }
	   break;
	 }
      }
parsedoptions:
  mystdout=stdout;signal(SIGPIPE,SIG_IGN);
  if(split)
   { oldstdout=dup(STDOUT);fclose(stdout);startprog((const char*Const*)argv);
     if(!minfields)			       /* no user specified minimum? */
	minfields=DEFminfields;				 /* take our default */
   }
  else if(every||digest||minfields)	      /* these combinations are only */
     goto usg;				  /* valid in combination with split */
  if((xheader||Xheader)&&logsummary||keepb&&!(areply||xheader))
usg:						     /* options sanity check */
   { elog(fmusage);					   /* impossible mix */
xusg:
     return EX_USAGE;
   }
  buf=malloc(buflen=BSIZE);totallen=0;i=maxindex(rex); /* prime some buffers */
  do rex[i].rexp=malloc(1);
  while(i--);
  fdate=0;addfield(&fdate,date,STRLEN(date)); /* fdate is only for searching */
  while((buflast=getchar())=='\n');		     /* skip leading garbage */
  if(!readhead())					    /* start looking */
   {
#ifdef sMAILBOX_SEPARATOR			      /* check for a leading */
     if(!strncmp(smboxsep,buf,STRLEN(smboxsep)))	/* mailbox separator */
      { buffilled=0;goto startover;				  /* skip it */
      }
#endif
     if(digest&&artheadr())
	goto startover;
   }
  else
startover:
     while(readhead());				 /* read in the whole header */
  ;{ size_t lenparkedbuf;void*parkedbuf;
     if(rdheader&&!strncmp(rdheader->fld_text,Article_,STRLEN(Article_)))
	rdheader->fld_text[STRLEN(Article_)-1]=HEAD_DELIMITER;	   /* proper */
     namep=0;totallen=0;i=maxindex(rex);			    /* field */
     do rex[i].rexl=0;
     while(i--);				 /* all state has been reset */
     for(fldp=rdheader;fldp;fldp=fldp->fld_next)    /* go through the linked */
      { int nowm;				    /* list of header-fields */
	if(conctenate)
	   concatenate(fldp);			 /* look for `sender' fields */
	chp=fldp->fld_text;j=fldp->id_len;i=maxindex(sest);
	while((sest[i].len!=j||strnIcmp(sest[i].head,chp,j))&&i--);
	if(i>=0&&(i!=maxindex(sest)||fldp==rdheader))	  /* found anything? */
	 { char*saddr;char*tmp;			     /* determine the weight */
	   nowm=trust?sest[i].wtrepl:areply?i:sest[i].wrepl;chp+=j;
	   saddr=tmp=malloc(j=fldp->tot_len-j);tmemmove(tmp,chp,j);
	   tmp[j-1]='\0';chp=pstrspn(tmp," \t\n");
	   for(saddr=0;;chp=skipwords(chp))		/* skip RFC 822 wise */
	    { switch(*chp)
	       { default:
		    if(!saddr)		   /* if we haven't got anything yet */
		       saddr=chp;		/* this might be the address */
		    continue;
		 case '<':skipwords(saddr=chp);	  /* hurray, machine useable */
		 case '\0':;
	       }
	      break;
	    }
	   if(saddr)			    /* any useful mailaddress found? */
	    { if(*saddr)			  /* did it have any length? */
	       { if(strstr(saddr,".UUCP"))
		    nowm-=(maxindex(sest)+2)*3;	 /* depreciate .UUCP address */
		 else if(!strpbrk(saddr,"@!/"))
		    nowm-=(maxindex(sest)+2)*2;		/* depreciate "user" */
		 else if(strchr(saddr,'@')&&!strchr(saddr,'.'))
		    nowm-=maxindex(sest)+2;	     /* depreciate user@host */
		 if(!namep||nowm>lastm)		/* better than previous ones */
		  { saddr=strcpy(malloc(strlen(saddr)+1),saddr);lastm=nowm;
		    goto newnamep;
		  }
	       }
	      else if(sest[i].head==returnpath)		/* nill Return-Path: */
	       { saddr=0;nowm=maxindex(sest)+2;			 /* override */
newnamep:	 if(namep)
		    free(namep);
		 namep=saddr;
	       }
	    }
	   free(tmp);
	 }				   /* save headers for later perusal */
	i=maxindex(rex);chp=fldp->fld_text;j=fldp->id_len;    /* e.g. areply */
	while((rex[i].lenr!=j||strnIcmp(rex[i].headr,chp,j))&&i--);
	chp+=j;
	if(i>=0&&(j=fldp->tot_len-j)>1)			  /* found anything? */
	   tmemmove(rex[i].rexp=realloc(rex[i].rexp,rex[i].rexl=j),chp,j);
      }
     tmemmove(parkedbuf=malloc(buffilled),buf,lenparkedbuf=buffilled);
     buffilled=0;    /* moved the contents of buf out of the way temporarily */
     if(areply)		      /* autoreply requested, we clean up the header */
      { for(fldp= *(afldp= &rdheader);fldp;)
	   if(!(fp2=findf(fldp,iheader))||fp2->id_len<fp2->tot_len-1)
	      *afldp=fldp->fld_next,free(fldp),fldp= *afldp;   /* remove all */
	   else					/* except the ones mentioned */
	      fldp= *(afldp= &fldp->fld_next);		       /* as -i ...: */
	loadbuf(to,STRLEN(to));loadchar(' ');	   /* generate the To: field */
	if(namep)	       /* did we find a valid return address at all? */
	   loadbuf(namep,strlen(namep));	      /* then insert it here */
	else
	   loadbuf(unknown,STRLEN(unknown));	    /* or insert our default */
	loadchar('\n');addbuf();		       /* add it to rdheader */
	if(subj->rexl)				      /* any Subject: found? */
	 { loadbuf(subject,STRLEN(subject));	  /* sure, check for leading */
	   if(strnIcmp(pstrspn(chp=subj->rexp," \t"),Re,STRLEN(Re)))  /* Re: */
	      loadbuf(re,STRLEN(re));	       /* no Re: , add one ourselves */
	   loadsaved(subj);addbuf();
	 }
	if(refr->rexl||msid->rexl)	   /* any References: or Message-ID: */
	 { loadbuf(references,STRLEN(references)); /* yes insert References: */
	   if(refr->rexl)
	    { if(msid->rexl)	    /* if we're going to append a Message-ID */
		 --refr->rexl;		    /* suppress the trailing newline */
	      loadsaved(refr);
	    }
	   if(msid->rexl)
	      loadsaved(msid);		       /* here's our missing newline */
	   addbuf();
	 }
	if(msid->rexl)			 /* do we add an In-Reply-To: field? */
	   loadbuf(inreplyto,STRLEN(inreplyto)),loadsaved(msid),addbuf();
      }				       /* are we allowed to add From_ lines? */
     else if(!force&&(!rdheader||!eqFrom_(rdheader->fld_text)))	 /* missing? */
      { struct field*old;time_t t;	     /* insert a From_ line up front */
	t=time((time_t*)0);old=rdheader;rdheader=0;
	loadbuf(From_,STRLEN(From_));
	if(namep)			  /* we found a valid return address */
	   loadbuf(namep,strlen(namep));
	else
	   loadbuf(unknown,STRLEN(unknown));			    /* Date: */
	if(!hdate->rexl||!findf(fdate,aheader))
	   loadchar(' '),chp=ctime(&t),loadbuf(chp,strlen(chp)); /* no Date: */
	else					 /* we generate it ourselves */
	   loadsaved(hdate);	      /* yes, found Date:, then copy from it */
	addbuf();rdheader->fld_next=old;
      }
     for(fldp=aheader;fldp;fldp=fldp->fld_next)
	if(!findf(fldp,rdheader))	       /* only add what didn't exist */
	   addfield(&nheader,fldp->fld_text,fldp->tot_len);
     if((fldp= *(afldp= &rdheader))&&logsummary&&eqFrom_(fldp->fld_text))
	concatenate(fldp),putssn(fldp->fld_text,fldp->tot_len);
     while(fldp)
      { lnl=fldp->id_len;chp=fldp->fld_text;
	if(logsummary)
	 { if(lnl==STRLEN(subject)&&!strnIcmp(chp,subject,lnl))
	    { concatenate(fldp);chp[i=fldp->tot_len-1]='\0';detab(chp);
	      putcs(' ');putssn(chp,i>=MAXSUBJECTSHOW?MAXSUBJECTSHOW:i);
	      putcs('\n');
	    }
	 }
	else if(findf(fldp,xheader))		   /* extract field contents */
	   putssn(chp+lnl,fldp->tot_len-lnl);
	else if(findf(fldp,Xheader))			   /* extract fields */
	   putssn(chp,fldp->tot_len);
	if(findf(fldp,Iheader))				    /* delete fields */
	 { *afldp=fldp->fld_next,free(fldp);fldp= *afldp;continue;
	 }
	else if(fp2=findf(fldp,Rheader))	  /* explicitly rename field */
	   renfield(afldp,lnl,fp2->fld_text+lnl,fp2->tot_len-lnl);
	else if((fp2=findf(fldp,iheader))&&!(areply&&lnl==fp2->tot_len-1))
	   renfield(afldp,(size_t)0,old_,STRLEN(old_)); /* implicitly rename */
	fldp= *(afldp= &(*afldp)->fld_next);
      }					/* restore the saved contents of buf */
     tmemmove(buf,parkedbuf,buffilled=lenparkedbuf);free(parkedbuf);
   }
  if(xheader||Xheader)			     /* we're just extracting fields */
     clearfield(&rdheader),clearfield(&nheader);	    /* throw it away */
  else			     /* otherwise, display the new & improved header */
   { flushfield(&rdheader);flushfield(&nheader);dispfield(Aheader);
     dispfield(iheader);dispfield(Iheader);lputcs('\n');  /* make sure it is */
   }						/* followed by an empty line */
  if(namep)
     free(namep);
  if(!keepb&&(areply||xheader||Xheader))		    /* decision time */
   { logfolder();				   /* we throw away the rest */
     if(split)
	closemine();
     opensink();					 /* discard the body */
   }
  lnl=1;					  /* last line was a newline */
  if(buffilled==1)		   /* the header really ended with a newline */
     buffilled=0;	      /* throw it away, since we already inserted it */
  while(buffilled||!lnl||buflast!=EOF)	 /* continue the quest, line by line */
   { if(!buffilled)				      /* is it really empty? */
	readhead();				      /* read the next field */
     if(rdheader)		    /* anything looking like a header found? */
      { if(eqFrom_(chp=rdheader->fld_text))	      /* check if it's From_ */
fromanyway:
	 { register size_t k;
	   if(split&&(lnl||every)&&    /* more thorough check for a postmark */
	    (k=strcspn(chp=pstrspn(chp+STRLEN(From_)," \t")," \t\n"))&&
	    *pstrspn(chp+k," \t")!='\n')
	      goto accuhdr;		     /* ok, postmark found, split it */
	   if(bogus)						   /* disarm */
	      lputcs(ESCAP);
	 }
	else if(split&&digest&&(lnl||every)&&digheadr())	  /* digest? */
accuhdr: { for(i=minfields;--i&&readhead()&&digheadr(););   /* found enough? */
	   if(!i)					   /* then split it! */
splitit:    { if(!lnl)	    /* did the previous mail end with an empty line? */
		 lputcs('\n');			      /* but now it does :-) */
	      logfolder();
	      if((fclose(mystdout)==EOF||errout==EOF)&&!quiet)
		 nlog(couldntw),elog(", continuing...\n"),split= -1;
	      if(!nowait)
		 waitforit();		 /* wait till the child has finished */
	      startprog((const char*Const*)argv);goto startover;
	    }					    /* and there we go again */
	 }
      }
     else if(eqFrom_(buf))			 /* special case, From_ line */
      { addbuf();goto fromanyway;      /* add it manually, readhead() didn't */
      }
     else if(split&&digest&&(lnl||every)&&artheadr())
	goto accuhdr;
#ifdef MAILBOX_SEPARATOR
     if(!strncmp(emboxsep,buf,STRLEN(emboxsep)))	     /* end of mail? */
      { if(split)		       /* gobble up the next start separator */
	 { buffilled=0;
#ifdef sMAILBOX_SEPARATOR
	   getline();buffilled=0;		 /* but only if it's defined */
#endif
	   if(buflast!=EOF)					   /* if any */
	      goto splitit;
	   break;
	 }
	else if(bogus)
	   goto putsp;				   /* escape it with a space */
      }
     else if(!strncmp(smboxsep,buf,STRLEN(smboxsep)&&bogus))
putsp:	lputcs(' ');
#endif /* MAILBOX_SEPARATOR */
     lnl=buffilled==1;		      /* check if we just read an empty line */
     if(areply&&bogus)					  /* escape the body */
	if(fldp=rdheader)	      /* we already read some "valid" fields */
	 { register char*p;
	   rdheader=0;
	   do			       /* careful, they can contain newlines */
	    { fp2=fldp->fld_next;chp=fldp->fld_text;
	      do lputcs(ESCAP),lputssn(chp,(p=strchr(chp,'\n')+1)-chp);
	      while((chp=p)<fldp->fld_text+fldp->tot_len);
	      free(fldp);					/* delete it */
	    }
	   while(fldp=fp2);		       /* escape all fields we found */
	 }
	else
	 { if(buffilled>1)	  /* we don't escape empty lines, looks neat */
	      lputcs(ESCAP);
	   goto flbuf;
	 }
     else if(rdheader)
	flushfield(&rdheader); /* beware, after this buf can still be filled */
     else
flbuf:	lputssn(buf,buffilled),buffilled=0;
   }			       /* make sure the mail ends with an empty line */
  logfolder();closemine();child= -1;waitforit();	/* wait for everyone */
  return split<0?EX_IOERR:EX_OK;
}

eqFrom_(a)const char*const a;
{ return!strncmp(a,From_,STRLEN(From_));
}

breakfield(line,len)const char*const line;size_t len;	   /* look where the */
{ const char*p=line;			   /* fieldname ends (RFC 822 specs) */
  if(eqFrom_(line))				      /* special case, From_ */
     return STRLEN(From_);
  while(len--&&!iscntrl(*p))		    /* no control characters allowed */
   { switch(*p++)
      { default:continue;
	case HEAD_DELIMITER:len=p-line;return len==1?0:len;	  /* eureka! */
	case ' ':;					/* no spaces allowed */
      }
     break;
   }
  return 0;		    /* sorry, does not seem to be a legitimate field */
}
