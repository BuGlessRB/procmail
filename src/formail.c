/************************************************************************
 *	formail - The mail (re)formatter				*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: formail.c,v 1.53 1994/06/28 16:56:10 berg Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 1994/06/28 16:56:10 $";
#include "includes.h"
#include <ctype.h>		/* iscntrl() */
#include "formail.h"
#include "acommon.h"
#include "sublib.h"
#include "shell.h"
#include "common.h"
#include "fields.h"
#include "ecommon.h"
#include "formisc.h"

#define ssl(str)		str,STRLEN(str)
#define bsl(str)		{ssl(str)}
#define sslbar(str,bar1,bar2)	{ssl(str),STRLEN(bar1)-1,STRLEN(bar2)-1}

static const char
#define X(name,value)	name[]=value,
#include "header.h"				  /* pull in the definitions */
#undef X
 From_[]=		FROM,				/* VNIX 'From ' line */
 Article_[]=		"Article ",		   /* USENET 'Article ' line */
 x_[]=			"X-",				/* general extension */
 old_[]=		OLD_PREFIX,			     /* my extension */
 xloop[]=		"X-Loop:",				/* ditto ... */
 unknown[]=UNKNOWN,re[]=" Re:",fmusage[]=FM_USAGE;

static const struct {const char*hedr;int lnr;}cdigest[]=
{
#define X(name,value)	bsl(name),
#include "header.h"		     /* pull in the precalculated references */
#undef X
};

/*
 *	sender determination fields in order of importance/reliability
 *	reply-address determination fields (wrepl specifies the weight
 *	for regular replies, wtrepl specifies the weight for trusted users)
 *
 *	I bet this is the first time you see a bar graph in C-source-code :-)
 */
static const struct {const char*head;int len,wrepl,wtrepl;}sest[]=
{ sslbar(replyto	,"******"	,"********"	),
  sslbar(Fromm		,"*"		,"*******"	),
  sslbar(retreceiptto	,"********"	,"*****"	),
  sslbar(sender		,"*****"	,"******"	),
  sslbar(res_replyto	,"***********"	,"***********"	),
  sslbar(res_from	,"***foo***"	,"***bar****"	),
  sslbar(res_sender	,"**********"	,"*********"	),
  sslbar(errorsto	,"*******"	,"****"		),
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
int errout,oldstdout,quiet=1,buflast,lenfileno;
long initfileno;
char ffileno[LEN_FILENO_VAR+8*sizeof(initfileno)*4/10+1+1]=DEFfileno;
int lexitcode;					     /* dummy, for waitfor() */
pid_t child= -1;
unsigned long rhash;
FILE*mystdout;
int nrskip,nrtotal= -1,retval=EX_OK;
size_t buflen,buffilled;
long totallen;
char*buf,*logsummary;
struct field*rdheader,*xheader,*Xheader,*uheader,*Uheader;
static struct field*iheader,*Iheader,*aheader,*Aheader,*Rheader,*nheader;

static void logfolder P((void))	 /* estimate the no. of characters needed to */
{ size_t i;charNUM(num,totallen);		       /* represent totallen */
  static const char tabchar[]=TABCHAR;
  if(logsummary)
   { putssn(sfolder,STRLEN(sfolder));putssn(logsummary,i=strlen(logsummary));
     i+=STRLEN(sfolder);i-=i%TABWIDTH;
     do putssn(tabchar,STRLEN(tabchar));
     while((i+=TABWIDTH)<LENoffset);
     ultstr(7,totallen,num);putssn(num,strlen(num));putcs('\n');
   }
}
    /* checks if the last field in rdheader looks like a known digest header */
static int digheadr P((void))
{ char*chp;int i;size_t j;struct field*fp;
  for(fp=rdheader;fp->fld_next;fp=fp->fld_next);	 /* skip to the last */
  i=maxindex(cdigest);chp=fp->fld_text;j=fp->id_len;
  while((cdigest[i].lnr!=j||strnIcmp(cdigest[i].hedr,chp,j))&&i--);
  return i>=0||j>STRLEN(old_)&&!strnIcmp(old_,chp,STRLEN(old_))||
   j>STRLEN(x_)&&!strnIcmp(x_,chp,STRLEN(x_));
}

static int artheadr P((void))	     /* could it be the start of an article? */
{ if(!rdheader&&!strncmp(buf,Article_,STRLEN(Article_)))
   { addbuf();rdheader->id_len=STRLEN(Article_);
     return 1;
   }
  return 0;
}

static PROGID;

main(lastm,argv)int lastm;const char*const argv[];
{ int i,split=0,force=0,bogus=1,every=0,areply=0,trust=0,digest=0,nowait=0,
   keepb=0,minfields=(char*)progid-(char*)progid,conctenate=0,babyl=0,
   babylstart;
  off_t maxlen,insoffs,ctlength;FILE*idcache=0;pid_t thepid;
  size_t j,lnl,escaplen;char*chp,*namep,*escap=ESCAP;
  struct field*fldp,*fp2,**afldp,*fdate,*fcntlength;
  if(lastm)			       /* sanity check, any argument at all? */
#define Qnext_arg()	if(!*chp&&!(chp=(char*)*++argv))goto usg
     while(chp=(char*)*++argv)
      { if((lastm= *chp++)==FM_SKIP)
	   goto number;
	else if(lastm!=FM_TOTAL)
	   goto usg;
	for(;;)
	 { switch(lastm= *chp++)
	    { case FM_TRUST:trust=1;
		 continue;
	      case FM_REPLY:areply=1;
		 continue;
	      case FM_FORCE:force=1;
		 continue;
	      case FM_EVERY:every=1;
		 continue;
	      case FM_BABYL:babyl=every=1;
	      case FM_DIGEST:digest=1;
		 continue;
	      case FM_NOWAIT:nowait=1;
		 continue;
	      case FM_KEEPB:keepb=1;
		 continue;
	      case FM_CONCATENATE:conctenate=1;
		 continue;
	      case FM_QUIET:quiet=1;
		 if(*chp=='-')
		    chp++,quiet=0;
		 continue;
	      case FM_LOGSUMMARY:Qnext_arg();
		 if(strlen(logsummary=chp)>MAXfoldlen)
		    chp[MAXfoldlen]='\0';
		 detab(chp);
		 break;
	      case FM_SPLIT:split=1;
		 if(!*chp)
		  { ++argv;
		    goto parsedoptions;
		  }
		 goto usg;
	      case HELPOPT1:case HELPOPT2:elog(fmusage);elog(FM_HELP);
		 goto xusg;
	      case FM_DUPLICATE:case FM_MINFIELDS:Qnext_arg();chp++;
	      default:chp--;
number:		 if(*chp-'0'>(unsigned)9)	    /* the number is not >=0 */
		    goto usg;
		 i=strtol(chp,&chp,10);
		 switch(lastm)			/* where does the number go? */
		  { case FM_SKIP:nrskip=i;
		       break;
		    case FM_DUPLICATE:maxlen=i;Qnext_arg();
		       if(!(idcache=fopen(chp,"r+b"))&&	  /* existing cache? */
			  !(idcache=fopen(chp,"w+b")))	    /* create cache? */
			{ nlog("Couldn't open");logqnl(argv[i]);
			  return EX_CANTCREAT;
			}
		       goto nextarg;
		    case FM_MINFIELDS:minfields=i;
		       break;
		    default:nrtotal=i;
		  }
		 continue;
	      case FM_BOGUS:bogus=0;
		 continue;
	      case FM_QPREFIX:Qnext_arg();escap=chp;
		 break;
	      case FM_ADD_IFNOT:case FM_ADD_ALWAYS:case FM_REN_INSERT:
	      case FM_DEL_INSERT:case FM_EXTRACT:case FM_EXTRC_KEEP:
	      case FM_FIRST_UNIQ:case FM_LAST_UNIQ:case FM_ReNAME:Qnext_arg();
		 i=breakfield(chp,lnl=strlen(chp));
		 switch(lastm)
		  { case FM_DEL_INSERT:case FM_REN_INSERT:case FM_EXTRACT:
		    case FM_FIRST_UNIQ:case FM_LAST_UNIQ:case FM_EXTRC_KEEP:
		       if(-i!=lnl)
		    default:
			  if(i<=0)
			     goto invfield;
		    case FM_ReNAME:;
		  }
		 chp[lnl]='\n';			       /* terminate the line */
		 afldp=addfield(lastm==FM_REN_INSERT?&iheader:
		  lastm==FM_DEL_INSERT?&Iheader:lastm==FM_ADD_IFNOT?&aheader:
		  lastm==FM_ADD_ALWAYS?&Aheader:lastm==FM_EXTRACT?&xheader:
		  lastm==FM_FIRST_UNIQ?&uheader:lastm==FM_LAST_UNIQ?&Uheader:
		  lastm==FM_EXTRC_KEEP?&Xheader:&Rheader,chp,++lnl);
		 if(lastm==FM_ReNAME)	      /* then we need a second field */
		  { int copied=0;
		    for(namep=(chp=(fldp= *afldp)->fld_text)+lnl,
		     chp+=lnl=fldp->id_len;chp<namep;++chp)
		     { switch(*chp)			  /* skip whitespace */
			{ case ' ':case '\t':case '\n':
			     continue;
			}
		       break;
		     }				   /* second field attached? */
		    lastm=i;
		    if((i=breakfield(chp,(size_t)(namep-chp)))>0)
		       tmemmove((char*)fldp->fld_text+lnl,chp,i),copied=1;
		    else if(namep>chp&&lastm<=0|| /* first field ended early */
			    !(chp=(char*)*++argv)||	 /* look at next arg */
			    (i=breakfield(chp,strlen(chp)))<=0) /* no field? */
invfield:	     { nlog("Invalid field-name:");logqnl(chp?chp:"");
		       goto usg;
		     }
		    *afldp=fldp=
		     realloc(fldp,FLD_HEADSIZ+(fldp->tot_len=lnl+i));
		    if(!copied)			   /* if not squeezed on yet */
		       tmemmove((char*)fldp->fld_text+lnl,chp,i);  /* do now */
		  }
	      case '\0':;
	    }
	   break;
	 }
nextarg:;
      }
parsedoptions:
  escaplen=strlen(escap);mystdout=stdout;signal(SIGPIPE,SIG_IGN);
  thepid=getpid();
  if(split)
   { char**ep;char**vfileno=0;
     for(ep=environ;*ep;ep++)		   /* gobble through the environment */
	if(!strncmp(*ep,ffileno,LEN_FILENO_VAR))	 /* look for FILENO= */
	   vfileno=ep;					    /* yes, found it */
     if(!vfileno)			/* FILENO= found in the environment? */
      { size_t envlen;						 /* no, pity */
	envlen=(ep-environ+1)*sizeof*environ;		   /* current length */
	tmemmove(ep=malloc(envlen+sizeof*environ),environ,envlen);
	*(vfileno=(char**)((char*)(environ=ep)+envlen))=0;*--vfileno=ffileno;
      }						      /* copy over the array */
     if((lenfileno=strlen(chp= *vfileno+LEN_FILENO_VAR))>
	STRLEN(ffileno)-LEN_FILENO_VAR-1)	  /* check the desired width */
	lenfileno=STRLEN(ffileno)-LEN_FILENO_VAR-1;	/* too big, truncate */
     if((initfileno=strtol(chp,&chp,10))<0)	  /* fetch the initial value */
	lenfileno--;				 /* correct it for negatives */
     if(*chp)						 /* no valid number? */
	lenfileno= -1;			    /* disable the FILENO generation */
     else
	*vfileno=ffileno;	    /* stuff our template in the environment */
     oldstdout=dup(STDOUT);fclose(stdout);
     if(!nrtotal)
	goto onlyhead;
     startprog((const char*Const*)argv);
     if(!minfields)			       /* no user specified minimum? */
	minfields=DEFminfields;				 /* take our default */
   }
  else if(nrskip>0||nrtotal>=0||every||digest||minfields||nowait)
     goto usg;			     /* only valid in combination with split */
  if((xheader||Xheader)&&logsummary||keepb&&!(areply||xheader||Xheader))
usg:						     /* options sanity check */
   { elog(fmusage);					   /* impossible mix */
xusg:
     return EX_USAGE;
   }
  buf=malloc(buflen=Bsize);totallen=0;i=maxindex(rex); /* prime some buffers */
  do rex[i].rexp=malloc(1);
  while(i--);
  fdate=0;addfield(&fdate,date,STRLEN(date)); /* fdate is only for searching */
  fcntlength=0;addfield(&fcntlength,cntlength,STRLEN(cntlength));   /* ditto */
  if(areply)					       /* when auto-replying */
     addfield(&iheader,xloop,STRLEN(xloop));	  /* preserve X-Loop: fields */
  if(babyl)						/* skip BABYL leader */
   { while(getchar()!=BABYL_SEP1||getchar()!=BABYL_SEP2||getchar()!='\n')
	while(getchar()!='\n');
     while(getchar()!='\n');
   }
  while((buflast=getchar())=='\n');		     /* skip leading garbage */
  if(!readhead())					    /* start looking */
   {
#ifdef sMAILBOX_SEPARATOR			      /* check for a leading */
     if(!strncmp(smboxsep,buf,STRLEN(smboxsep)))	/* mailbox separator */
      { buffilled=0;						  /* skip it */
	goto startover;
      }
#endif
     if(digest&&artheadr())
	goto startover;
   }
  else
startover:
     while(readhead());				 /* read in the whole header */
  ;{ size_t lenparkedbuf;void*parkedbuf;
     if(rdheader)
      { char*tmp,*tmp2;
	if(!strncmp(tmp=(char*)rdheader->fld_text,Article_,STRLEN(Article_)))
	   tmp[STRLEN(Article_)-1]=HEAD_DELIMITER;
	else if(babyl&&
		!force&&
		!strncmp(tmp,mailfrom,STRLEN(mailfrom))&&
		eqFrom_(tmp2=skpspace(tmp+STRLEN(mailfrom))))
	 { rdheader->id_len=STRLEN(From_);
	   tmemmove(tmp,tmp2,rdheader->tot_len-=tmp2-tmp);
	 }
      }
     namep=0;totallen=0;i=maxindex(rex);
     do rex[i].rexl=0;
     while(i--);
     clear_uhead(uheader);clear_uhead(Uheader);	 /* all state has been reset */
     for(fldp=rdheader;fldp;fldp=fldp->fld_next)    /* go through the linked */
      { int nowm;				    /* list of header-fields */
	if(conctenate)
	   concatenate(fldp);			 /* look for `sender' fields */
	chp=fldp->fld_text;j=fldp->id_len;i=maxindex(sest);
	while((sest[i].len!=j||strnIcmp(sest[i].head,chp,j))&&i--);
	if(i>=0&&(i!=maxindex(sest)||fldp==rdheader))	  /* found anything? */
	 { char*saddr;char*tmp;			     /* determine the weight */
	   nowm=trust?sest[i].wtrepl:areply?sest[i].wrepl:i;chp+=j;
	   tmp=malloc(j=fldp->tot_len-j);tmemmove(tmp,chp,j);
	   (chp=tmp)[j-1]='\0';
	   if(sest[i].head==From_)
	    { char*pastad;
	      if(trust||!(saddr=strchr(chp,'\n')))   /* skip the first line? */
		 saddr=chp;					  /* no need */
	      if(*saddr=='\n'&&(pastad=strchr(saddr,' ')))
		 saddr=pastad+1;		/* reposition at the address */
	      chp=saddr;
	      while((pastad=strchr(chp,'\n'))&&(pastad=strchr(pastad,' ')))
		 chp=pastad+1;		      /* skip to the last uucp >From */
	      if(pastad=strchr(chp,' '))		/* found an address? */
	       { char*savetmp;				      /* lift it out */
		 savetmp=malloc((j=pastad-chp)+1);tmemmove(savetmp,chp,j);
		 savetmp[j]='\0';
		 if(strchr(savetmp,'@'))		 /* domain attached? */
		    chp=savetmp,savetmp=tmp,tmp=chp;		/* ok, ready */
		 else				/* no domain, bang away! :-) */
		  { static const char remf[]=" remote from ",
		     fwdb[]=" forwarded by ";
		    char*p1,*p2;
		    chp=tmp;
		    for(;;)
		     { int c;
		       p1=strstr(saddr,remf);
		       if(!(p2=strstr(saddr,fwdb))&&!p1)
			  break;			     /* no more info */
		       if(!p1||p2&&p2<p1)	      /* pick the first bang */
			  p1=p2+STRLEN(fwdb);
		       else
			  p1+=STRLEN(remf);
		       for(;;)				     /* copy it over */
			{ switch(c= *p1++)
			   { default:*chp++=c;
				continue;
			     case '\0':case '\n':*chp++='!'; /* for the buck */
			   }
			  break;
			}
		       saddr=p1;			/* continue the hunt */
		     }
		    strcpy(chp,savetmp);chp=tmp;     /* attach the user part */
		  }
		 free(savetmp);	  /* (temporary buffers might have switched) */
	       }
	    }
	   while(*(chp=skpspace(chp))=='\n')
	      chp++;
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
	       { if(!strpbrk(saddr,"@!/"))
		    nowm-=(maxindex(sest)+2)*4;		/* depreciate "user" */
		 else if(strstr(saddr,".UUCP"))
		    nowm-=(maxindex(sest)+2)*3;	 /* depreciate .UUCP address */
		 else if(strchr(saddr,'@')&&!strchr(saddr,'.'))
		    nowm-=(maxindex(sest)+2)*2;	     /* depreciate user@host */
		 else if(strchr(saddr,'!'))
		    nowm-=(maxindex(sest)+2)*1;	     /* depreciate bangpaths */
		 if(!namep||nowm>lastm)		/* better than previous ones */
		  { saddr=strcpy(malloc(strlen(saddr)+1),saddr);lastm=nowm;
		    goto newnamep;
		  }
	       }
	      else if(sest[i].head==returnpath)		/* nill Return-Path: */
	       { saddr=0;lastm=maxindex(sest)+2;		 /* override */
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
	 { tmemmove(rex[i].rexp=realloc(rex[i].rexp,(rex[i].rexl=j)+1),chp,j);
	   rex[i].rexp[j]='\0';			     /* add a terminating \0 */
	 }
      }
     if(idcache)
      { int dupid=0;
	if(msid->rexl)					/* any Message-ID: ? */
	 { insoffs=maxlen;msid->rexp[msid->rexl-1]='\0';
	   do					/* wipe out trailing newline */
	    { int j;char*p;	  /* start reading & comparing the next word */
	      for(p=msid->rexp;(j=fgetc(idcache))==*p;p++)
		 if(!j)					     /* end of word? */
		  { if(!quiet)
		       nlog("Duplicate ID found:"),elog(msid->rexp),elog("\n");
		    dupid=1;
		    goto dupfound;		     /* YES! duplicate found */
		  }
	      if(!j)					     /* end of word? */
	       { if(p==msid->rexp&&insoffs==maxlen)	 /* first character? */
		  { insoffs=ftell(idcache)-1;		     /* found end of */
		    goto skiprest;			  /* circular buffer */
		  }
	       }
	      else
skiprest:	 for(;;)			/* skip the rest of the word */
		  { switch(fgetc(idcache))
		     { case EOF:
			  goto noluck;
		       default:
			  continue;
		       case '\0':;
		     }
		    break;
		  }
	    }
	   while(ftell(idcache)<maxlen);		  /* past our quota? */
noluck:	   if(insoffs>=maxlen)				  /* past our quota? */
	      insoffs=0;			     /* start up front again */
	   fseek(idcache,insoffs,SEEK_SET);
	   fwrite(msid->rexp,1,msid->rexl+1,idcache);
dupfound:  fseek(idcache,(off_t)0,SEEK_SET);	 /* rewind, for any next run */
	   msid->rexp[msid->rexl-1]='\n';	      /* restore the newline */
	 }
	if(!split)			  /* not splitting?  terminate early */
	   return dupid?EX_OK:1;
	if(dupid)			       /* duplicate? suppress output */
	   closemine(),opensink();
      }
     ctlength=0;
     if(!digest&&(fldp=findf(fcntlength,&rdheader)))
      { *(chp=(char*)fldp->fld_text+fldp->tot_len-1)='\0';   /* terminate it */
	ctlength=strtol((char*)fldp->fld_text+STRLEN(cntlength),(char**)0,10);
	*chp='\n';			     /* restore the trailing newline */
      }
     tmemmove(parkedbuf=malloc(buffilled),buf,lenparkedbuf=buffilled);
     buffilled=0;    /* moved the contents of buf out of the way temporarily */
     if(areply)		      /* autoreply requested, we clean up the header */
      { for(fldp= *(afldp= &rdheader);fldp;)
	   if(!(fp2=findf(fldp,&iheader))||fp2->id_len<fp2->tot_len-1)
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
	   if(strnIcmp(skpspace(chp=subj->rexp),Re,STRLEN(Re)))	      /* Re: */
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
	   loadbuf(unknown,STRLEN(unknown));
	loadchar(' ');				   /* insert one extra blank */
	if(!hdate->rexl||!findf(fdate,&aheader))		    /* Date: */
	   loadchar(' '),chp=ctime(&t),loadbuf(chp,strlen(chp)); /* no Date: */
	else					 /* we generate it ourselves */
	   loadsaved(hdate);	      /* yes, found Date:, then copy from it */
	addbuf();rdheader->fld_next=old;
      }
     for(fldp=aheader;fldp;fldp=fldp->fld_next)
	if(!findf(fldp,&rdheader))	       /* only add what didn't exist */
	   if(fldp->id_len+1>=fldp->tot_len&&		  /* field name only */
	      (fldp->id_len==STRLEN(messageid)&&
	       !strnIcmp(fldp->fld_text,messageid,STRLEN(messageid))||
	       fldp->id_len==STRLEN(res_messageid)&&
	       !strnIcmp(fldp->fld_text,res_messageid,STRLEN(res_messageid))))
	    { char*p;const char*name;unsigned long h1,h2,h3;
	      static unsigned long h4; /* conjure up a `unique' msg-id field */
	      h1=time((time_t*)0);h2=thepid;h3=rhash;
	      p=chp=malloc(fldp->id_len+2+1+((sizeof h1*8+5)/6+1)*4+1+
	       strlen(name=hostname())+2);     /* allocate worst case length */
	      strncpy(p,fldp->fld_text,fldp->id_len);*(p+=fldp->id_len)=' ';
	      *++p='<';*++p='"';*(p=ultoan(h3,p+1))='.';
	      *(p=ultoan(h4,p+1))='.';*(p=ultoan(h2,p+1))='.';
	      *(p=ultoan(h1,p+1))='"';*++p='@';strcpy(p+1,name);
	      *(p=strchr(p,'\0'))='>';*++p='\n';addfield(&nheader,chp,p-chp+1);
	      free(chp);h4++;					/* put it in */
	    }
	   else
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
	if(findf(fldp,&Iheader))			    /* delete fields */
	   goto delfld;
	;{ struct field*uf;
	   if((uf=findf(fldp,&uheader))&&!uf->fld_ref)
	      uf->fld_ref=afldp;		   /* first uheader, keep it */
	   else if(fp2=findf(fldp,&Uheader))
	    { if(fp2->fld_ref)
	       { if(afldp==&(*fp2->fld_ref)->fld_next)
		    afldp=fp2->fld_ref;
		 delfield(fp2->fld_ref);	       /* delete old Uheader */
	       }
	      fp2->fld_ref=afldp;			/* keep last Uheader */
	    }
	   else if(uf)			    /* delete all following uheaders */
delfld:	    { fldp=delfield(afldp);
	      continue;
	    }
	 }
	if(fp2=findf(fldp,&Rheader))		  /* explicitly rename field */
	   renfield(afldp,lnl,(char*)fp2->fld_text+lnl,fp2->tot_len-lnl);
	else if((fp2=findf(fldp,&iheader))&&!(areply&&lnl==fp2->tot_len-1))
	   renfield(afldp,(size_t)0,old_,STRLEN(old_)); /* implicitly rename */
	fldp= *(afldp= &(*afldp)->fld_next);
      }					/* restore the saved contents of buf */
     tmemmove(buf,parkedbuf,buffilled=lenparkedbuf);free(parkedbuf);
   }
  flushfield(&rdheader);flushfield(&nheader);dispfield(Aheader);
  dispfield(iheader);dispfield(Iheader);
  if(namep)
     free(namep);
  if(keepb||!(xheader||Xheader))	 /* we're not just extracting fields */
     lputcs('\n');		/* make sure it is followed by an empty line */
  if(!keepb&&(areply||xheader||Xheader))		    /* decision time */
   { logfolder();				   /* we throw away the rest */
     if(split)
	closemine();
     else		      /* terminate early, only the header was needed */
	goto onlyhead;
     opensink();					 /* discard the body */
   }
  lnl=1;					  /* last line was a newline */
  if(buffilled==1)		   /* the header really ended with a newline */
     buffilled=0;	      /* throw it away, since we already inserted it */
  if(babyl)
   { int c,lc;					/* ditch pseudo BABYL header */
     for(lc=0;c=getchar(),c!=EOF&&(c!='\n'||lc!='\n');lc=c);
     babylstart=0;
   }
  if(ctlength>0)
   { if(buffilled)
	lputssn(buf,buffilled),ctlength-=buffilled,buffilled=lnl=0;
     ;{ int tbl=buflast,lwr='\n';
	while(--ctlength>=0&&tbl!=EOF)	       /* skip Content-Length: bytes */
	   lnl=lwr==tbl&&lwr=='\n',putcs(lwr=tbl),tbl=getchar();
	if((buflast=tbl)=='\n'&&lwr!=tbl)	/* just before a line break? */
	   putcs('\n'),buflast=getchar();		/* wrap up loose end */
      }
     if(!quiet&&ctlength>0)
      { charNUM(num,ctlength);
	nlog(cntlength);elog(" field exceeds actual length by ");
	ultstr(0,(unsigned long)ctlength,num);elog(num);elog(" bytes\n");
      }
   }
  while(buffilled||!lnl||buflast!=EOF)	 /* continue the quest, line by line */
   { if(!buffilled)				      /* is it really empty? */
	readhead();				      /* read the next field */
     if(!babyl||babylstart)	       /* don't split BABYL files everywhere */
      { if(rdheader)		    /* anything looking like a header found? */
	 { if(eqFrom_(chp=rdheader->fld_text))	      /* check if it's From_ */
fromanyway: { register size_t k;
	      if(split&&
		 (lnl||every)&&	       /* more thorough check for a postmark */
		 (k=strcspn(chp=skpspace(chp+STRLEN(From_))," \t\n"))&&
		 *skpspace(chp+k)!='\n')
		 goto accuhdr;		     /* ok, postmark found, split it */
	      if(bogus)						   /* disarm */
		 lputssn(escap,escaplen);
	    }
	   else if(split&&digest&&(lnl||every)&&digheadr())	  /* digest? */
accuhdr:    { for(i=minfields;--i&&readhead()&&digheadr();); /* found enough */
	      if(!i)					   /* then split it! */
splitit:       { if(!lnl)   /* did the previous mail end with an empty line? */
		    lputcs('\n');		      /* but now it does :-) */
		 logfolder();
		 if(fclose(mystdout)==EOF||errout==EOF)
		  { split= -1;
		    if(!quiet)
		       nlog(couldntw),elog(", continuing...\n");
		  }
		 if(!nowait&&*argv)	 /* wait till the child has finished */
		  { int excode;
		    if((excode=waitfor(child))!=EX_OK&&retval!=EX_OK)
		       retval=excode;
		  }
		 if(!nrtotal)
		    goto onlyhead;
		 startprog((const char*Const*)argv);
		 goto startover;
	       }				    /* and there we go again */
	    }
	 }
	else if(eqFrom_(buf))			 /* special case, From_ line */
	 { addbuf();		       /* add it manually, readhead() didn't */
	   goto fromanyway;
	 }
	else if(split&&digest&&(lnl||every)&&artheadr())
	   goto accuhdr;
      }
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
     if(babyl&&*buf==BABYL_SEP1)
	babylstart=1,closemine(),opensink();		 /* discard the rest */
     if(areply&&bogus)					  /* escape the body */
	if(fldp=rdheader)	      /* we already read some "valid" fields */
	 { register char*p;
	   rdheader=0;
	   do			       /* careful, they can contain newlines */
	    { fp2=fldp->fld_next;chp=fldp->fld_text;
	      do
	       { lputssn(escap,escaplen);
		 lputssn(chp,(p=strchr(chp,'\n')+1)-chp);
	       }
	      while((chp=p)<(char*)fldp->fld_text+fldp->tot_len);
	      free(fldp);					/* delete it */
	    }
	   while(fldp=fp2);		       /* escape all fields we found */
	 }
	else
	 { if(buffilled>1)	  /* we don't escape empty lines, looks neat */
	      lputssn(escap,escaplen);
	   goto flbuf;
	 }
     else if(rdheader)
      { struct field*ox,*oX;
	ox=xheader;oX=Xheader;xheader=Xheader=0;flushfield(&rdheader);
	xheader=ox;Xheader=oX; /* beware, after this buf can still be filled */
      }
     else
flbuf:	lputssn(buf,buffilled),buffilled=0;
   }			       /* make sure the mail ends with an empty line */
  logfolder();
onlyhead:
  closemine();
  ;{ int excode;					/* wait for everyone */
     while((excode=waitfor((pid_t)0))!=NO_PROCESS)
	if(retval==EX_OK&&excode!=EX_OK)
	   retval=excode;
   }
  if(retval<0)
     retval=EX_UNAVAILABLE;
  return retval!=EX_OK?retval:split<0?EX_IOERR:EX_OK;
}

eqFrom_(a)const char*const a;
{ return !strncmp(a,From_,STRLEN(From_));
}

int breakfield(line,len)const char*const line;size_t len;  /* look where the */
{ const char*p=line;			   /* fieldname ends (RFC 822 specs) */
  if(eqFrom_(p))				      /* special case, From_ */
     return STRLEN(From_);
  while(len&&!iscntrl(*p))		    /* no control characters allowed */
   { switch(*p++)
      { default:len--;
	   continue;
	case HEAD_DELIMITER:len=p-line;
	   return len==1?0:len;					  /* eureka! */
	case ' ':p--;					/* no spaces allowed */
      }
     break;
   }
  return -(int)(p-line);    /* sorry, does not seem to be a legitimate field */
}
