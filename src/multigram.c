/************************************************************************
 *	multigram - The human mail address reader			*
 *									*
 *	It uses multigrams to intelligently filter out mail addresses	*
 *	from the garbage in any arbitrary mail.				*
 *	Multigram is currently unable to pick out addresses that	*
 *	contain embedded whitespace.					*
 *	This program also contains some swiss-army-knife mailinglist	*
 *	support features.						*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: multigram.c,v 1.22 1993/02/10 17:08:06 berg Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 1993/02/10 17:08:06 $";
#include "includes.h"
#include "sublib.h"
#include "shell.h"
#include "ecommon.h"

#define BUFSTEP		16
#define COPYBUF		16384
/*#define SPEEDBUF	COPYBUF	       /* uncomment to get a speed increase? */
#define SCALE_WEIGHT	0x7fff

#define DEFmaxgram	4
#define DEFminweight	(SCALE_WEIGHT/4)	      /* sanity cutoff value */
#define DEFbest_matches 2

#define PROCMAIL	"../.bin/procmail"	  /* some configurable paths */
#define DEFAULTS_DIR	".etc"
#define GLOCKFILE	"../.etc/rc.lock"
#define RCMAIN		"./.etc/rc.main"
#define LLOCKFILE	"rc.lock"
#define REQUEST		"-request"
#define RCSUBMIT	"./rc.submit"
#define RCREQUEST	"./rc.request"
#define RCPOST		"./../.etc/rc.post"
#define RCINIT		"RC_INIT=rc.init"
#define XENVELOPETO	"X_ENVELOPE_TO="
#define LIST		"list="

#define metoo_SENDMAIL		"-om"
#define nometoo_SENDMAIL	"-omF"
#define REMOV1_DELIM "(Only"
#define REMOV2_DELIM "addresses below this line can be automatically removed)"
#define NOT_METOO	"(-n)"

struct string{char*text,*itext;size_t buflen;};

static remov_delim;

strnIcmp(a,b,l)const char*a,*b;size_t l;			     /* stub */
{ return strncmp(a,b,l);
}
		    /* read a string from a file into a struct string buffer */
static size_t readstr(file,p,linewise)FILE*const file;struct string*p;
 const int linewise;
{ size_t len;int i,firstspc;
  static const char rem1str[]=REMOV1_DELIM,rem2str[]=REMOV2_DELIM;
  for(len=firstspc=0;;)
   { switch(i=getc(file))
      { case ' ':case '\t':case '\n':
	   if(!len)				  /* only skip leading space */
	      continue;
	   if(!linewise)		      /* do we need a complete line? */
	      break;				       /* no, a word will do */
	   if(!firstspc)			     /* already seen spaces? */
	    { if(i=='\n')			     /* no, so check for EOL */
	       { p->text[len]='\0';	  /* terminate the first word, split */
		 if(++len==p->buflen)		 /* still buffer space left? */
		    p->text=realloc(p->text,p->buflen+=BUFSTEP);
		 break;
	       }
	      i='\0';firstspc=1;
	    }			 /* not the first word on the line, continue */
	   if(i=='\n')
	      break;
	default:p->text[len]=i;		      /* regular character, store it */
	   if(++len==p->buflen)			   /* watch our buffer space */
	      p->text=realloc(p->text,p->buflen+=BUFSTEP);
	   continue;					   /* next character */
	case EOF:;
      }
     p->text[len]='\0';			 /* terminate the buffer in any case */
     if(linewise&&!remov_delim&&!strcmp(p->text,rem1str)&&
      !strcmp(p->text+sizeof rem1str,rem2str))	       /* special delimiter? */
	remov_delim=1;
     return len;
   }
}

static char*tstrdup(p)const char*const p;
{ return strcpy(malloc(strlen(p)+1),p);
}

static void lowcase(str)struct string*const str;	   /* make lowercase */
{ register char*p;
  for(p=str->itext=tstrdup(str->text);*p;p++)
     if((unsigned)*p-'A'<'Z'-'A')
	*p+='a'-'A';
}

static void elog(a)const char*const a;
{ fputs(a,stderr);
}
							/* the program names */
static const char idhash[]="idhash",flist[]="flist",dirsep[]=DIRSEP;
static const char*progname="multigram";

void nlog(a)const char*const a;		    /* log error with identification */
{ elog(progname);elog(": ");elog(a);
}
						 /* finds the next character */
static char*lastdirsep(filename)const char*filename;
{ const char*p;					/* following the last DIRSEP */
  while(p=strpbrk(filename,dirsep))
     filename=p+1;
  return(char*)filename;
}
						   /* check rc.lock file age */
static void rclock(file,stbuf)const char*const file;struct stat*const stbuf;
{ while(!stat(file,stbuf)&&time((time_t*)0)-stbuf->st_mtime<DEFlocktimeout)
     sleep(DEFlocksleep);			     /* wait, if appropriate */
}

static char*argstr(first,last)const char*first,*last;		/* construct */
{ char*chp;size_t i;				   /* an argument assignment */
  strcpy(chp=malloc((i=strlen(first))+strlen(last)+1),first);
  strcpy(chp+i,last);return chp;
}

static PROGID;

main(minweight,argv)char*argv[];
{ struct string fuzzstr,hardstr;FILE*hardfile;const char*addit=0;
  struct match{char*fuzz,*hard;int metric;long lentry,offs1,offs2;}
   **best,*curmatch=0;
  unsigned best_matches,maxgram,maxweight,charoffs=0,remov=0,renam=0,
   chkmetoo=(char*)progid-(char*)progid;
  int lastfrom;
  static const char usage[]=
 "Usage: multigram [-cdmr] [-b nnn] [-l nnn] [-w nnn] [-a address] filename\n";
  if(minweight)			      /* sanity check, any arguments at all? */
   { char*chp;
     if(!strcmp(chp=lastdirsep(argv[0]),flist))		 /* suid flist prog? */
      { struct stat stbuf;
	progname=flist;*chp='\0';	    /* security check, 3 hardlinks!? */
	if(!chdir(argv[0])&&!lstat(flist,&stbuf)&&S_ISREG(stbuf.st_mode)&&
	 stbuf.st_mode&S_ISUID&&stbuf.st_uid==geteuid()&&stbuf.st_nlink==3&&
	 !chdir(chPARDIR))
	 { static const char request[]=REQUEST,xenvlpto[]=XENVELOPETO,
	    rcrequest[]=RCREQUEST,rcpost[]=RCPOST,list[]=LIST,
	    *pmexec[]={PROCMAIL,RCSUBMIT,RCINIT,0,0,rcrequest,rcpost,0};
#define Endpmexec(i)	(pmexec[maxindex(pmexec)-(i)])
	   char*arg;
	   if(minweight!=2)		       /* wrong number of arguments? */
	    { elog("Usage: flist listname[-request]\n");return EX_USAGE;
	    }
	   chp=strchr(arg=argv[1],'\0');	       /* check for -request */
	   if(chp-arg>STRLEN(request)&&!strcmp(chp-=STRLEN(request),request))
	      *chp='\0',pmexec[1]=rcrequest,Endpmexec(1)=0,Endpmexec(2)=rcpost;
	   else
	      chp=0;
	   if(chdir(arg))		     /* goto the list's subdirectory */
	      pmexec[1]=RCMAIN,Endpmexec(2)=0,chdir(DEFAULTS_DIR);
	   Endpmexec(4)=argstr(list,arg);	    /* pass on the list name */
	   if(chp)				  /* was it a -request list? */
	      *chp= *request;		     /* then restore the leading '-' */
	   Endpmexec(3)=argstr(xenvlpto,arg);setuid(stbuf.st_uid);
	   setgid(stbuf.st_gid);rclock(GLOCKFILE,&stbuf);	    /* stall */
	   rclock(LLOCKFILE,&stbuf);
	   execve(pmexec[0],(char*const*)pmexec,environ);  /* start procmail */
	   nlog("Couldn't exec \"");elog(pmexec[0]);elog("\"\n");
	   return EX_UNAVAILABLE;				    /* panic */
	 }
	nlog("Missing permissions\n");return EX_NOPERM;
      }
     setgid(getgid());setuid(getuid());		  /* revoke suid permissions */
     if(!strcmp(chp,idhash))				  /* idhash program? */
      { unsigned long hash=0;int i;
	progname=idhash;
	if(minweight!=1)
	 { elog("Usage: idhash\n");return EX_USAGE;
	 }
	while(i=fgetc(stdin),!feof(stdin))		       /* hash away! */
	   hash=hash*67067L+i;
	printf("%lx",hash);return EX_OK;
      }
     minweight=SCALE_WEIGHT;best_matches=maxgram=0;
     while((chp= *++argv)&&*chp=='-')
	for(chp++;;)
	 { int c;
	   switch(c= *chp++)
	    { case 'c':charoffs=1;continue;
	      case 'd':remov=1;continue;
	      case 'r':renam=1;continue;
	      case 'm':chkmetoo=1;continue;
	      case 'a':
		 if(!*chp&&!(chp= *++argv))
		    goto usg;
		 addit=chp;break;
	      case 'b':case 'l':case 'w':
	       { int i;
		 ;{ const char*ochp;
		    if(!*chp&&!(chp= *++argv)||
		     (i=strtol(ochp=chp,(char**)&chp,10),chp==ochp))
		       goto usg;
		  }
		 switch(c)
		  { case 'b':best_matches=i;continue;
		    case 'l':minweight=i;continue;
		    case 'w':maxgram=i;continue;
		  }
	       }
	      case HELPOPT1:case HELPOPT2:elog(usage);
		 elog(
 "\t-a address\tadd this address to the list\
\n\t-b nnn\t\tmaximum no. of best matches shown\
\n\t-c\t\tdisplay offsets in characters\
\n\t-d\t\tdelete address from list\
\n\t-m\t\tcheck for metoo\
\n\t-l nnn\t\tlower bound metric\
\n\t-r\t\trename address on list\
\n\t-w nnn\t\twindow width used when matching\n");return EX_USAGE;
	      case '-':
		 if(!*chp)
		  { chp= *++argv;goto lastopt;
		  }
	      default:goto usg;
	      case '\0':;
	    }
	   break;
	 }
lastopt:
     if(!chp||*++argv||renam+remov+!!addit>1)
	goto usg;
     if(!(hardfile=fopen(chp,remov||renam||addit?"r+":"r")))
      { nlog("Couldn't open \"");elog(chp);elog("\"\n");return EX_IOERR;
      }
#ifdef SPEEDBUF				   /* allocate a bigger stdio buffer */
     setvbuf(hardfile,malloc(SPEEDBUF),_IOFBF,(size_t)SPEEDBUF);
#endif
   }
  else
usg:
   { elog(usage);return EX_USAGE;
   }
  if(addit)			      /* special subfunction, to add entries */
   { int lnl;long lasttell;				 /* to the dist file */
     for(lnl=1,lasttell=0;;)
      { switch(getc(hardfile))			    /* step through the file */
	 { case '\n':
	      if(!lnl)			    /* looking for trailing newlines */
		 lnl=1,lasttell=ftell(hardfile);
	      continue;
	   default:lnl=0;continue;
	   case EOF:;				   /* or the end of the file */
	 }
	break;
      }				     /* go back there, and add the new entry */
     fseek(hardfile,lasttell,SEEK_SET);fprintf(hardfile,"%s\n",addit);
     printf("Added: %s\n",addit);fclose(hardfile);return EX_OK;
   }
  if(!maxgram)
     maxgram=DEFmaxgram;
  maxgram--;
  if(minweight==SCALE_WEIGHT)
     minweight=DEFminweight;
  if(!best_matches)
     best_matches=DEFbest_matches;
  fuzzstr.text=malloc(fuzzstr.buflen=BUFSTEP);
  hardstr.text=malloc(hardstr.buflen=BUFSTEP);
  ;{ int i;
     best=malloc(best_matches--*sizeof*best);i=best_matches;
     do
      { best[i]=malloc(sizeof**best);best[i]->hard=malloc(1);
	best[i]->fuzz=malloc(1);best[i]->metric= -SCALE_WEIGHT;
      }
     while(i--);
   }
  for(lastfrom= -1;readstr(stdin,&fuzzstr,0);)
   { int meter,maxmetric;size_t fuzzlen;long linentry,offs1,offs2;
     ;{ char*chp,*echp;int parens;
	echp=strchr(chp=fuzzstr.text,'\0')-1;
	do
	 { switch(*echp)
	    { case '.':case ',':case ';':case ':':case '?':case '!':*echp='\0';
		 continue;
	    }
	   break;
	 }
	while(--echp>chp);    /* roughly check if it could be a mail address */
	if(lastfrom<=0&&!strpbrk(chp,"@/")&&(!strchr(chp,'!')||
	 strchr(chp,'|')||strchr(chp,',')||strstr(chp,"..")))
	 { if(lastfrom<0)
	      lastfrom=!strcmp(SHFROM,chp);
	   continue;			  /* apparently not an email address */
	 }
	lastfrom=0;
	for(parens=0;chp=strchr(chp,'(');chp++,parens++);
	for(chp=fuzzstr.text;chp=strchr(chp,')');chp++,parens--);
	if(*(chp=fuzzstr.text)=='(')
	 { if(!parens&&*echp==')')
	    { *echp='\0';goto shftleft;
	    }
	   if(parens>0)
shftleft:     tmemmove(chp,chp+1,strlen(chp));
	 }
	else if(parens<0&&*echp==')')
	   *echp='\0';
	if(*(chp=fuzzstr.text)=='<'&&*(echp=strchr(chp,'\0')-1)=='>'
	 &&!strchr(chp,','))			      /* strip '<' and '>' ? */
	   *echp='\0',tmemmove(chp,chp+1,echp-chp);
	if(!(fuzzlen=strlen(chp)))
	   continue;
	;{ int i=0;
	   do
	    { if(best[i]->metric==-SCALE_WEIGHT&&!strcmp(best[i]->fuzz,chp))
		 break;
	      if(!strcmp(best[i]->fuzz,chp))	/* already matched this one? */
		 goto dupl_addr;
	    }
	   while(++i<=best_matches);
	 }
	if(!curmatch)
	   curmatch=malloc(sizeof*curmatch);
	curmatch->fuzz=tstrdup(chp);curmatch->hard=malloc(1);
	curmatch->metric= -SCALE_WEIGHT;
      }
     lowcase(&fuzzstr);fseek(hardfile,0L,SEEK_SET);
     maxmetric=best[best_matches]->metric;
     for(remov_delim=offs2=linentry=0;
      offs1=offs2,readstr(hardfile,&hardstr,1);)
      { size_t minlen,hardlen,maxlen;register size_t gramsize;
	offs2=ftell(hardfile);linentry++;
	if(*hardstr.text=='(')
	   continue;				   /* unsuitable for matches */
	lowcase(&hardstr);
	if((minlen=hardlen=strlen(hardstr.text))>(maxlen=fuzzlen))
	   minlen=fuzzlen,maxlen=hardlen;
	if((gramsize=minlen-1)>maxgram)
	   gramsize=maxgram;
	maxweight=SCALE_WEIGHT/(gramsize+1);
	meter=(int)((unsigned long)SCALE_WEIGHT/2*minlen/maxlen)-
	 SCALE_WEIGHT/2;
	do				    /* reset local multigram counter */
	 { register lmeter=0;size_t cmaxlen=maxlen;
	   ;{ register const char*fzz,*hrd;
	      fzz=fuzzstr.itext;
	      do
	       { for(hrd=fzz+1;hrd=strchr(hrd,*fzz);)	 /* is it present in */
		    if(!strncmp(++hrd,fzz+1,gramsize))	      /* own string? */
		     { if(cmaxlen>gramsize+1)
			  cmaxlen--;
		       goto dble_gram;		     /* skip until it's last */
		     }
		 for(hrd=hardstr.itext;hrd=strchr(hrd,*fzz);)	/* otherwise */
		    if(!strncmp(++hrd,fzz+1,gramsize))	 /* search it in the */
		     { lmeter++;break;			       /* dist entry */
		     }
dble_gram:;    }
	      while(*(++fzz+gramsize));				/* next gram */
	    }
	   if(lmeter)
	    { unsigned weight;
	      if(cmaxlen>minlen)
		 cmaxlen=minlen;
	      meter+=lmeter*(weight=maxweight/(unsigned)(cmaxlen-gramsize));
	      meter-=weight*
	       (unsigned long)((lmeter+=gramsize-cmaxlen)<0?-lmeter:lmeter)/
	       cmaxlen;
	    }
	 }
	while(gramsize--);		 /* search all gramsizes down to one */
	free(hardstr.itext);			 /* check if we had any luck */
	if(meter>maxmetric&&(remov_delim||!renam&&!remov))
	 { curmatch->metric=maxmetric=meter;curmatch->lentry=linentry;
	   free(curmatch->hard);hardlen++;
	   hardlen+=strlen(hardstr.text+hardlen)+1;
	   curmatch->hard=malloc(hardlen+=strlen(hardstr.text+hardlen)+1);
	   tmemmove(curmatch->hard,hardstr.text,hardlen);
	   curmatch->offs1=offs1;curmatch->offs2=offs2;
	 }
      }
     free(fuzzstr.itext);	 /* maybe this match can be put in the array */
     if(curmatch->metric>-SCALE_WEIGHT)		   /* of best matches so far */
      { struct match*mp,**mmp;
	free((mp= *(mmp=best+best_matches))->fuzz);free(mp->hard);free(mp);
	while(--mmp>=best&&(mp= *mmp)->metric<curmatch->metric)
	   mmp[1]=mp;					   /* keep it sorted */
	mmp[1]=curmatch;curmatch=0;
      }
     else
	free(curmatch->fuzz),free(curmatch->hard);
dupl_addr:;
   }
  ;{ int i;struct match*mp;
     for(i=0;i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	if(chkmetoo)
	   printf("%s\n",strcmp(mp->hard+strlen(mp->hard)+1,NOT_METOO)
	    ?metoo_SENDMAIL:nometoo_SENDMAIL);
	else
	   printf("%3ld %-34s %5d %-34s\n",
	    charoffs?mp->offs1:mp->lentry,mp->hard,mp->metric,mp->fuzz);
     if((mp= *best)->metric>=minweight)
      { struct match*worse;
	if(renam)
	 { long line;int i,w1;
	   maxweight>>=1;
	   for(i=1,line=mp->lentry,w1=mp->metric,worse=0;
	    i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	      if(mp->lentry==line&&mp->metric+maxweight<w1)
	       { goto remv1;
	       }
	   for(i=1;i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	      if(mp->metric+maxweight<w1)
remv1:	       { worse=mp;mp= *best;goto remv;
	       }
	   nlog("Couldn't find a proper address pair\n");goto norenam;
	 }
	if(remov)
remv:	 { char*buf;long offs1,offs2;size_t readin;
	   buf=malloc(COPYBUF);offs1=mp->offs1;offs2=mp->offs2;
	   while(fseek(hardfile,offs2,SEEK_SET),
	    readin=fread(buf,1,COPYBUF,hardfile))
	    { offs2=ftell(hardfile);fseek(hardfile,offs1,SEEK_SET);
	      if(buf[readin-1]=='\n')	  /* try and remove some empty lines */
		 while(readin>1&&buf[readin-2]=='\n')	/* at the end, since */
		    readin--;		     /* every time could be the last */
	      fwrite(buf,1,readin,hardfile);offs1=ftell(hardfile);
	    }
	   free(buf);fseek(hardfile,offs1,SEEK_SET);
	   printf("Removed: %s\n",mp->hard);
	   if(renam)
	      fputs(worse->fuzz,hardfile),printf("Added: %s\n",worse->fuzz);
	   do putc('\n',hardfile);			   /* erase the tail */
	   while(ftell(hardfile)<offs2);
	   fclose(hardfile);
	 }
	return EX_OK;
      }
   }
  if(remov||renam)
   { nlog("Couldn't even find a single address\n");
   }
norenam:
  return 1;
}
