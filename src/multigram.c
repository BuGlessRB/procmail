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
 *	Most notably:	flist	A program that should be setuid root	*
 *	or setuid to the list user and setgid to the list user's	*
 *	primary group.							*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1992-1999, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: multigram.c,v 1.94 2000/06/12 04:52:36 guenther Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 2000/06/12 04:52:36 $";
#include "includes.h"
#include "sublib.h"
#include "hsort.h"
#include "shell.h"
#include "ecommon.h"
#include "mcommon.h"
#include "lastdirsep.h"
#include "../patchlevel.h"

#include "targetdir.h"	    /* see ../SmartList/install.sh2 for more details */

#define BUFSTEP		16
#define ADDR_INCR	128
#define COPYBUF		16384
/*#define SPEEDBUF	COPYBUF	       /* uncomment to get a speed increase? */
#define SCALE_WEIGHT	0x7fff
#define EXCL_THRESHOLD	30730

#define DEFmaxgram	4
#define DEFminweight	(SCALE_WEIGHT/4)	      /* sanity cutoff value */
#define DEFbest_matches 2

#define TMPMAILFILE	"/tmp/choplist.%ld"

#define DEFAULTS_DIR	".etc"			  /* some configurable paths */
#define GLOCKFILE	"../.etc/rc.lock"
#define RCMAIN		"./../.etc/rc.main"
#define LLOCKFILE	"rc.lock"
#define LINKMOVED	"../.etc/Moved"
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

struct string{char*text,*itext;size_t textlen,buflen;};

const char dirsep[]=DIRSEP;
static remov_delim,maxgram;

int strnIcmp(a,b,l)const char*a,*b;size_t l;			     /* stub */
{ return strncmp(a,b,l);
}

static off_t tcoffset;
static tcguessed;

static void ctellinit P((void))
{ tcoffset=tcguessed=0;
}

static off_t ctell(fp)FILE*fp;				  /* caching ftell() */
{ if(tcguessed==3)			/* three good guesses for confidence */
     return tcoffset;			       /* eliminated another lseek() */
  ;{ off_t offset;
     if((offset=ftell(fp))==tcoffset)
	tcguessed++;				       /* got this one right */
     return offset;
   }
}
		    /* read a string from a file into a struct string buffer */
static size_t readstr(file,p,linewise)FILE*const file;struct string*p;
 const int linewise;
{ size_t tccount,len;int i,firstspc;
  static const char rem1str[]=REMOV1_DELIM,rem2str[]=REMOV2_DELIM;
  for(tccount=len=firstspc=0;;)
   { tccount++;
     switch(i=getc(file))
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
     tcoffset+=tccount;
     for(;p->text[len]='\0',len;--len)	     /* terminate buffer in any case */
      { switch(p->text[len-1])			     /* trailing whitespace? */
	 { case ' ':case '\t':				      /* wipe it out */
	      continue;
	 }
	break;
      }
     if(linewise&&len)
	for(i=0;!remov_delim&&!i;i=1)
	   if(!strcmp(p->text+i,rem1str)&&
	    !strcmp(p->text+sizeof rem1str+i,rem2str)) /* special delimiter? */
	      remov_delim=1;
     return len;
   }
}

static char*tstrdup(p)const char*const p;
{ return strcpy(malloc(strlen(p)+1),p);
}

static const char*mailfile;
static int retval=EX_UNAVAILABLE;

static void sterminate P((void))
{ unlink(mailfile);exit(retval);
}

static int strIcmp(s1,s2)const char*const s1,*const s2;
{ register unsigned i,j;register const char*a,*b;
  a=s1;b=s2;
  do
   { while(*a&&*a==*b)
	a++,b++;
     if((i= *a++)-'A'<='Z'-'A')
	i+='a'-'A';
     if((j= *b++)-'A'<='Z'-'A')
	j+='a'-'A';
     if(j!=i)
	return i>j?a-s1:s1-a;
   }
  while(i&&j);
  return 0;
}

static int pstrIcmp(s1,s2)const void*const s1,*const s2;
{ return strIcmp(*(const char*const*)s1,*(const char*const*)s2);
}

static void revstr(p)register char*p;		       /* reverse the string */
{ register char*q;
  for(q=strchr(p,'\0')-1;p<q;p++,q--)
   { unsigned i;
     i= *p;*p= *q;*q=i;
   }
}

static void castlower(str)register char*str;
{ for(;*str;str++)
     if((unsigned)*str-'A'<='Z'-'A')
	*str+='a'-'A';
}

static char*findatlast(str)const char*str;
{ const char*p;
  for(p=0;str=strchr(str,'@');p=str++);		   /* find the last '@' sign */
  return (char*)p;
}

static void lowcase(str)struct string*const str;	   /* make lowercase */
{ char*p,*q;
  if(p=findatlast(str->text))
   { size_t l,l1;
     q=malloc((l=strlen(str->text))+1);
     *((char*)tmemmove(q,p,l1=str->text+(int)l-p)+l)='\0';
     tmemmove(q+l1,str->text,l-l1);	    /* swap the sides around the '@' */
   }				     /* this improves the boundary behaviour */
  else
     q=tstrdup(str->text);
  castlower(str->itext=q);
}

static void elog(a)const char*const a;
{ fputs(a,stderr);
}
							/* the program names */
static const char idhash[]="idhash",flist[]="flist",senddigest[]="senddigest",
 choplist[]="choplist";
static const char*progname="multigram";

#define ISPROGRAM(curname,refname)	\
 (!strncmp(curname,refname,STRLEN(refname)))

void nlog(a)const char*const a;		    /* log error with identification */
{ fprintf(stderr,"%s: %s",progname,a);
}

void logqnl(a)const char*const a;
{ fprintf(stderr," \"%s\"\n",a);
}
						   /* check rc.lock file age */
static int rclock(file,stbuf)const char*const file;struct stat*const stbuf;
{ int waited=0;
  while(!stat(file,stbuf)&&time((time_t*)0)-stbuf->st_mtime<DEFlocktimeout)
     waited=1,sleep(DEFlocksleep);		     /* wait, if appropriate */
  return waited;
}

static char*argstr(first,last)const char*first,*last;		/* construct */
{ char*chp;size_t i;				   /* an argument assignment */
  strcpy(chp=malloc((i=strlen(first))+strlen(last)+1),first);
  strcpy(chp+i,last);
  return chp;
}

static void checkparens(pleft,pright,str,echp)int pleft,pright;
 char*str,*const echp;
{ int parens;char*chp;
  for(chp=str,parens=0;chp=strchr(chp,pleft);chp++,parens++);
  for(chp=str;chp=strchr(chp,pright);chp++,parens--);
  if(*(chp=str)==pleft)				       /* any opening paren? */
   { if(!parens&&*echp==pright)		    /* parens matched and enclosing? */
      { *echp='\0';
	goto shftleft;
      }
     if(parens>0)			/* more opening than closing parens? */
shftleft:
	tmemmove(chp,chp+1,echp-chp+2);			/* delete left paren */
   }
  else if(parens<0&&*echp==pright)	/* more closing than opening parens? */
     *echp='\0';				       /* delete right paren */
}

static PROGID;

static int matchgram(fuzzstr,hardstr)
const struct string*const fuzzstr;struct string*const hardstr;
{ size_t minlen,maxlen;unsigned maxweight;int meter;
  register size_t gramsize;
  if((minlen=hardstr->textlen=strlen(hardstr->text))>(maxlen=fuzzstr->textlen))
     minlen=fuzzstr->textlen,maxlen=hardstr->textlen;
  if((gramsize=minlen-1)>maxgram)
     gramsize=maxgram;
  maxweight=SCALE_WEIGHT/(gramsize?gramsize:1);		/* distribute weight */
  meter=(int)((unsigned long)SCALE_WEIGHT/2*minlen/maxlen)-SCALE_WEIGHT/2;
  do					    /* reset local multigram counter */
   { register lmeter=0;size_t cmaxlen=maxlen;
     ;{ register const char*fzz,*hrd;
	fzz=fuzzstr->itext;
	do
	 { for(hrd=fzz+1;hrd=strchr(hrd,*fzz);)		 /* is it present in */
	      if(!strncmp(++hrd,fzz+1,gramsize))	      /* own string? */
	       { if(cmaxlen>gramsize+1)
		    cmaxlen--;
		  goto dble_gram;		     /* skip until it's last */
	       }
	   for(hrd=hardstr->itext;hrd=strchr(hrd,*fzz);)	/* otherwise */
	       if(!strncmp(++hrd,fzz+1,gramsize))	 /* search it in the */
		{ lmeter++;				       /* dist entry */
		  break;
		}
dble_gram:;
	 }
	while(*(++fzz+gramsize));				/* next gram */
      }
     if(lmeter)
      { unsigned weight;
	if(cmaxlen>minlen)
	   cmaxlen=minlen;
	meter+=lmeter*(weight=maxweight/(unsigned)(cmaxlen-gramsize));
	meter-=weight*
	 (unsigned long)((lmeter+=gramsize-cmaxlen)<0?-lmeter:lmeter)/cmaxlen;
      }
   }
  while(1<gramsize--);			 /* search all gramsizes down to two */
  return meter;
}

int main(argc,argv)int argc;char*argv[];
{ struct string fuzzstr,hardstr,excstr,exc2str;FILE*hardfile,**hfiles;
  const char*addit=0,*ldomain=0;char**nargv;size_t lldomain;
  struct match{char*fuzz,*hard;int metric;long lentry;off_t offs1,offs2;
   FILE*hardfile;}
   **best,*curmatch=0;
  unsigned best_matches,charoffs=0,fremov=0,remov=0,renam=0,multiple=0,
   incomplete=(char*)progid-(char*)progid;
  int lastfrom,minweight;static unsigned dodomain;
  static const char cldntopen[]="Couldn't open";
  static const char usage[]=
"Usage: multigram [-cdimr] [-b nnn] [-l nnn] [-w nnn] [-ax address]\n\
\t[-L domain] file ...\n"
   ;
  if(argc)			      /* sanity check, any arguments at all? */
   { char*chp;						 /* suid flist prog? */
     if(ISPROGRAM(chp=lastdirsep(argv[0]),flist))
      { struct stat stbuf;struct passwd*pass;char*arg;
	static const char request[]=REQUEST,listid[]=LISTID,
	 rcrequest[]=RCREQUEST,rcpost[]=RCPOST,list[]=LIST,
	 defdir[]=DEFAULTS_DIR,targetdir[]=TARGETDIR,
	 *pmexec[]={PROCMAIL,RCSUBMIT,RCINIT,0,0,0,rcrequest,rcpost,0};
#define Endpmexec(i)	(pmexec[maxindex(pmexec)-(i)])
	progname=flist;*chp='\0';
	if(argc!=2)			       /* wrong number of arguments? */
	 { elog("\
Usage: flist listname[-request]\n\
   Or: flist -v\n");
	   return EX_USAGE;
	 }
	if(!strcmp(arg=argv[1],"-v"))
	 { fprintf(stderr,"SmartList%s\nUser: %s\nDirectory: %s\n",VERSION,
	    listid,targetdir);
	   return EXIT_SUCCESS;
	 }
	;{ uid_t euid;
	   if((euid=geteuid())==ROOT_uid)
	    { if(!(pass=getpwnam(listid)))
	       { nlog("User \"");elog(listid);elog("\"");
		 goto bailout;
	       }
	     /*
	      * continue as the compile-time-determined list maintainer
	      */
	      if(setgid(pass->pw_gid)||initgroups(listid,pass->pw_gid)||
	       setuid(pass->pw_uid))		 /* this can fail on certain */
		 return EX_OSERR;	 /* broken systems e.g. Linux 2.2.14 */
	    }
	   else if(!(pass=getpwuid(euid)))
	    { nlog("Euid");fprintf(stderr," %d",(int)euid);
bailout:      elog(" unknown\n");
	      return EX_NOUSER;
	    }
	   else
	     /*
	      * we weren't root, so try to get the uid and gid belonging to
	      * the euid we started under
	      */
	    { int error;
	      setrgid(pass->pw_gid);error=setgid(pass->pw_gid);setruid(euid);
	      if(setuid(euid)||
		 error||
		 getuid()!=euid||
		 getgid()!=pass->pw_gid)
	       { nlog("Insufficient privileges to become");
		 logqnl(pass->pw_name);
		 return EX_NOPERM;
	       }
	    }
	   endpwent();
	   if(chdir(chp=pass->pw_dir))
	      goto nochdir;
	 }
	if(*targetdir&&chdir(chp=(char*)targetdir))
nochdir: { nlog("Couldn't chdir to");logqnl(chp);
	   return EX_NOINPUT;
	 }
	if(stat(defdir,&stbuf))
	 { nlog("Can't find \"");elog(defdir);elog("\" in");logqnl(chp);
	   return EX_NOINPUT;
	 }
	if(pass->pw_uid!=stbuf.st_uid||pass->pw_gid!=stbuf.st_gid)
	   nlog("Strange group or user id\n");	       /* check for -request */
	if((chp=strchr(arg,'\0'))-arg>STRLEN(request)&&
	   !strcmp(chp-=STRLEN(request),request))
	   *chp='\0',pmexec[1]=rcrequest,Endpmexec(1)=0,Endpmexec(2)=rcpost;
	else
	   chp=0;
	if(!strcmp(arg,chPARDIR)||strpbrk(arg,dirsep))
	 { nlog("Bogus listname\n");
	   return EX_DATAERR;
	 }
	;{ int foundlock;
	   do
	    { foundlock=0;
	      if(chdir(arg))		     /* goto the list's subdirectory */
		 pmexec[1]=RCMAIN,Endpmexec(2)=0,chdir(defdir);
	      while(rclock(GLOCKFILE,&stbuf)||rclock(LLOCKFILE,&stbuf))
		 foundlock=1;					    /* stall */
	    }
	   while(foundlock&&!chdir(LINKMOVED));	      /* did the lists move? */
	 }
	Endpmexec(5)=INIT_PATH;
	Endpmexec(4)=argstr(list,arg);		    /* pass on the list name */
	if(chp)					  /* was it a -request list? */
	   *chp= *request;		     /* then restore the leading '-' */
	Endpmexec(3)=argstr(XENVELOPETO,arg);
	execv(pmexec[0],(char*const*)pmexec);nlog("Couldn't exec");
	logqnl(pmexec[0]);
	return EX_UNAVAILABLE;					    /* panic */
      }
    /*
     *	revoke any suid permissions now, since we're not flist
     */
     if((setgid(getgid())&&getgid()!=getegid())||
      (setuid(getuid())&&getuid()!=geteuid()))
	return EX_OSERR;		       /* this _really_ can't happen */
     if(ISPROGRAM(chp,idhash))				  /* idhash program? */
      { unsigned long hash=0;int i;
	progname=idhash;
	if(argc!=1)
	 { elog("Usage: idhash\n");
	   return EX_USAGE;
	 }
	while(i=fgetc(stdin),!feof(stdin))		       /* hash away! */
	   hash=hash*67067L+i;
	printf("%lx",hash);
	return EXIT_SUCCESS;
      }
     if(ISPROGRAM(chp,senddigest))		      /* senddigest program? */
      { struct stat stbuf;
	progname=senddigest;
	if(argc<5)
	 { elog(
	 "Usage: senddigest maxage maxsize bodyfile trailerfile [file] ...\n");
	   return EX_USAGE;
	 }
	if(!stat(argv[3],&stbuf))
	 { time_t newt;off_t size;
	   newt=stbuf.st_mtime;size=stbuf.st_size;
	   if(!stat(argv[argc=4],&stbuf))
	    { off_t maxsize;
	      if(stbuf.st_mtime+strtol(argv[1],(char**)0,10)-newt<=0)
		 return EXIT_SUCCESS;			   /* digest too old */
	      maxsize=strtol(argv[2],(char**)0,10);
	      goto statd;
	      do
	       { if(!stat(argv[argc],&stbuf))
statd:		    if((size+=stbuf.st_size)>maxsize)	  /* digest too big? */
		       return EXIT_SUCCESS;
	       }
	      while(argv[++argc]);
	    }
	 }
	return 1;
      }
     hardstr.text=malloc(hardstr.buflen=BUFSTEP);
     if(ISPROGRAM(chp,choplist))
      { unsigned long minnames,mindiffnames,maxnames,maxsplits,maxsize,
	 maxconcur,maxensize;
	char*distfile,**revarr;int mailfd;size_t revfilled;
	static const char tmpmailfile[]=TMPMAILFILE;
	char lmailfile[STRLEN(TMPMAILFILE)+8*sizeof(pid_t)*4/10+1+1];
	progname=choplist;
	if(argc<9)
	 { elog(
"Usage: choplist minnames mindiffnames maxnames maxsplits maxsize maxconcur\n\
\tdistfile sendmail [flags ...]\n");
	   return EX_USAGE;
	 }
	minnames=strtol(argv[1],(char**)0,10);
	mindiffnames=strtol(argv[2],(char**)0,10);
	maxnames=strtol(argv[3],(char**)0,10);
	maxsplits=strtol(argv[4],(char**)0,10);
	maxsize=strtol(argv[5],(char**)0,10);
	maxconcur=strtol(argv[6],(char**)0,10);distfile=argv[7];
	nargv=argv+8;argc-=8;setbuf(stdin,(char*)0);setbuf(stdout,(char*)0);
	sprintf((char*)(mailfile=lmailfile),tmpmailfile,(long)getpid());
	qsignal(SIGTERM,sterminate);qsignal(SIGINT,sterminate);
	qsignal(SIGHUP,sterminate);qsignal(SIGQUIT,sterminate);
#ifdef SIGCHLD
	signal(SIGCHLD,SIG_DFL);
#endif
	signal(SIGPIPE,SIG_DFL);unlink(mailfile);
#ifndef O_CREAT
	if(0>(mailfd=creat(mailfile,NORMperm)))
#else
	if(0>(mailfd=open(mailfile,O_RDWR|O_CREAT|O_EXCL,NORMperm)))
#endif
	 { nlog("Can't create temporary file");logqnl(mailfile);
	   return EX_CANTCREAT;
	 }
	;{ char*buf;int i;unsigned long totsize=0;
	   buf=malloc(COPYBUF);
	   goto jin;
	   do
	    { char*a=buf;size_t len;
	      totsize+=(len=i);
	      do
	       { while(0>(i=write(mailfd,buf,(size_t)len))&&errno==EINTR);
		 if(i<0)
		  { nlog("Can't write temporary file");logqnl(mailfile);
		    retval=EX_IOERR;sterminate();
		  }
		 a+=i;
	       }
	      while(i>0&&(len-=i));
jin:	      while(0>(i=read(STDIN,buf,(size_t)COPYBUF))&&errno==EINTR);
	    }
	   while(i>0);
	   if(!totsize)
	      nlog("Can't find the mail\n"),retval=EX_NOINPUT,sterminate();
	   free(buf);totsize=(maxsize+totsize-1)/totsize;
	   if(maxsize&&(!maxsplits||totsize<maxsplits))
	      maxsplits=totsize?totsize:1;
	 }
	fclose(stdin);close(STDIN);
	if(!(hardfile=fopen(chp=distfile,"r")))
	   nlog(cldntopen),logqnl(distfile),retval=EX_IOERR,sterminate();
	;{ size_t revlen;
	   revarr=malloc((revlen=ADDR_INCR)*sizeof*revarr);revfilled=0;
	   while(readstr(hardfile,&hardstr,1))
	    { int i=strchr((chp=strchr(hardstr.text,'\0'))+1,'\0')[-1];
	      if(*hardstr.text!='(')
		 switch(i)
		  { default:
		       goto invaddr;
		    case ')':case '\0':;
		  }
	      else					     /* comment line */
		 switch(i)		      /* DomainOS compiler chokes on */
		  { default:	    /* labels between the switch() and the { */
invaddr:	       nlog("Skipping invalid address entry:");*chp=' ';
		       logqnl(hardstr.text);
		    case ')':case '\0':
		       continue;
		  }
	      if(revfilled==revlen)			  /* watch our space */
		 revarr=realloc(revarr,(revlen+=ADDR_INCR)*sizeof*revarr);
	      revstr(hardstr.text);revarr[revfilled++]=tstrdup(hardstr.text);
	    }
	 }
	free(hardstr.text);fclose(hardfile);
	if(!revfilled)
	   retval=EXIT_SUCCESS,sterminate();	  /* no recipients, finished */
	if(fork()>0)					    /* lose our tail */
	   return EXIT_SUCCESS;	  /* causes procmail to release the lockfile */
	revarr=realloc(revarr,revfilled*sizeof*revarr);		/* be modest */
	hsort(revarr,revfilled,sizeof*revarr,pstrIcmp);		  /* sort'em */
	if(maxsplits)
	 { maxsplits=(revfilled+maxsplits-1)/maxsplits;
	   if(!minnames||minnames<maxsplits)
	    { minnames=maxsplits;
	      if(maxnames&&maxnames<minnames)
		 maxnames=minnames+mindiffnames;
	    }
	 }
	if(!maxnames||maxnames>MAX_argc-argc)
	   maxnames=MAX_argc-argc;
	;{ size_t envc;const char*const*nam;
	   nam=(const char*const*)environ;envc=argc;
#define MAX_envc	0		  /* should be dynamic in the future */
	   for(maxensize=(MAX_argc+MAX_envc)*16L;*nam;
	    envc++,maxensize-=strlen(*nam++)+1+sizeof*nam);
	   if(maxnames>MAX_argc+MAX_envc-envc)
	      maxnames=MAX_argc+MAX_envc-envc;
	 }
	if(minnames>maxnames)
	   minnames=maxnames;
	if(!minnames)
	   minnames=1;
	;{ int*rdist,*ip;char**nam;size_t n;
	   ip=rdist=malloc((n=revfilled)*sizeof*rdist);nam=revarr;
	   while(--n)
	    { int i,j;char*left,*right;
	      left= *nam;
	      if(!(i=strIcmp(right= *++nam,left)))
		 j=SCALE_WEIGHT;	  /* identical!	 don't split them up */
	      else
		 for(j=0;--i;)
		    switch(*right++)
		     { case '@':j=SCALE_WEIGHT/2;	   /* domains match! */
		       case '.':j++;			   /* domain borders */
		     }
	      revstr(left);*ip++=j;
	    }
	   revstr(*nam);*ip=0;nam=malloc(1+ ++argc*sizeof*argv);
	   tmemmove(nam+1,nargv,argc*sizeof*argv);*(nargv=nam)=BinSh;
	   ;{ unsigned long cnames,cnsize,cconcur;
	      char**first,**best;
	      ;{ unsigned long maxnsize;
		 for(maxnsize=0;*nam;maxnsize+=strlen(*nam++)+1+sizeof*nam);
		 maxensize-=maxnsize;
		 if(maxensize<(maxnsize=MAX_argc-maxnsize))
		    maxensize=maxnsize;
	       }
	      n=cconcur=0;
	      do
	       { int bestval;
		 cnsize=strlen(*(first=nam=revarr+n))+1+sizeof*nam;cnames=0;
		 do
		  { if(nam-first<minnames||
		       bestval>=SCALE_WEIGHT/2&&rdist[n]>=SCALE_WEIGHT/2||
		       rdist[n]<bestval)
		       bestval=rdist[n],best=nam;
		    cnames++;
		  }
		 while(++n<revfilled&&
		       maxensize>=(cnsize+=strlen(*++nam)+1+sizeof*nam)&&
		       maxnames>cnames);
		 nam=(nargv=realloc(nargv,
		  (1+(bestval=best-first+1)+argc)*sizeof*argv))+argc;
		 if(maxconcur&&maxconcur<++cconcur)
		    wait((int*)0);
		 tmemmove(nam,first,bestval*sizeof*argv);nam[bestval]=0;
		 if(STDIN!=open(mailfile,O_RDONLY))
		  { nlog("Lost");logqnl(mailfile);retval=EX_NOINPUT;
		    sterminate();
		  }
		 for(;;)
		  { switch(fork())
		     { case -1:nlog("Couldn't fork, retrying\n");
			  if(wait((int*)0)==-1)
			     sleep(DEFsuspend);
			  continue;
		       case 0:				  /* may be a script */
			  execv(nargv[1],nargv+1);execv(nargv[0],nargv);
			  kill(getppid(),SIGTERM);nlog("Couldn't exec");
			  logqnl(nargv[0]);
			  return EX_UNAVAILABLE;
		     }
		    break;
		  }
		 close(STDIN);
	       }
	      while((n=best-revarr+1)<revfilled);
	    }
	 }
	retval=EXIT_SUCCESS;sterminate();
      }
     minweight=SCALE_WEIGHT;best_matches=maxgram=0;exc2str.text=excstr.text=0;
     nargv=argv;
     while((chp= *++nargv)&&*chp=='-')
	for(chp++;;)
	 { int c;
	   switch(c= *chp++)
	    { case 'c':charoffs=1;
		 continue;
	      case 'D':fremov=1;
	      case 'd':remov=1;
		 continue;
	      case 'i':incomplete=1;
		 continue;
	      case 'r':renam=1;
		 continue;
	      case 'm':multiple=1;
		 continue;
	      case 'a':
		 if(!*chp&&!(chp= *++nargv))
		    goto usg;
		 addit=chp;
		 break;
	      case 'L':
		 if(!*chp&&!(chp= *++nargv))
		    goto usg;
		 lldomain=strlen(ldomain=chp)+1;
		 break;
	      case 'x':
		 if(!*chp&&!(chp= *++nargv))
		    goto usg;
		 if(excstr.text)
		    exc2str.text=chp;
		 else
		    excstr.text=chp;
		 break;
	      case 'b':case 'l':case 'w':
	       { int i;
		 ;{ const char*ochp;
		    if(!*chp&&!(chp= *++nargv)||
		     (i=strtol(ochp=chp,(char**)&chp,10),chp==ochp))
		       goto usg;
		  }
		 switch(c)
		  { case 'b':best_matches=i;
		       continue;
		    case 'l':minweight=i;
		       continue;
		    case 'w':maxgram=i;
		       continue;
		  }
	       }
	      case HELPOPT1:case HELPOPT2:elog(usage);
		 elog(
 "\t-a address\tadd this address to the list\
\n\t-b nnn\t\tmaximum no. of best matches shown\
\n\t-c\t\tdisplay offsets in characters\
\n\t-d\t\tgently delete address from list\
\n\t-D\t\tforce delete address from list\
\n\t-i\t\tcheck for incomplete addresses too\
\n\t-m\t\tdisplay multiple matches per address\
\n\t-l nnn\t\tlower bound metric\
\n\t-L domain\tdefault domain for local addresses\
\n\t-r\t\trename address on list\
\n\t-x address\texclude this address from the search (max. 2)\
\n\t-w nnn\t\twindow width used when matching\n");
		 return EX_USAGE;
	      case '-':
		 if(!*chp)
		  { chp= *++nargv;
		    goto lastopt;
		  }
	      default:
		 goto usg;
	      case '\0':;
	    }
	   break;
	 }
lastopt:
     if(!chp||*++nargv&&addit||renam+remov+!!addit>1)
	goto usg;
     if(excstr.text)
      { excstr.textlen=strlen(excstr.text);lowcase(&excstr);
	if(exc2str.text)
	   exc2str.textlen=strlen(exc2str.text),lowcase(&exc2str);
      }
     hfiles=malloc((argc+1-(nargv-argv))*sizeof*hfiles);
     ;{ const char*accstr=remov||renam||addit?"r+":"r";
	unsigned i;
	if(!(*hfiles=hardfile=fopen(chp,accstr)))
	 { nlog(cldntopen);logqnl(chp);
	   return EX_IOERR;
	 }
	for(i=1;*nargv;nargv++)
	   if(hfiles[i]=fopen(*nargv,accstr))
	      i++;
	argc=i;
      }
#ifdef SPEEDBUF				   /* allocate a bigger stdio buffer */
     setvbuf(hardfile,malloc(SPEEDBUF),_IOFBF,(size_t)SPEEDBUF);
#endif
   }
  else
usg:
   { elog(usage);
     return EX_USAGE;
   }
  if(addit)			      /* special subfunction, to add entries */
   { int lnl;off_t lasttell;				 /* to the dist file */
     for(lnl=1,lasttell=0;;)
      { switch(getc(hardfile))			    /* step through the file */
	 { case '\n':
	      if(!lnl)			    /* looking for trailing newlines */
		 lnl=1,lasttell=ftell(hardfile);
	      continue;
	   default:lnl=0;
	      continue;
	   case EOF:;				   /* or the end of the file */
	 }
	break;
      }				     /* go back there, and add the new entry */
     fseek(hardfile,lasttell,SEEK_SET);fprintf(hardfile,"%s\n",addit);
     printf("Added: %s\n",addit);
     return EXIT_SUCCESS;
   }
  if(!maxgram)
     maxgram=DEFmaxgram;
  maxgram--;
  if(minweight==SCALE_WEIGHT)
     minweight=DEFminweight;
  if(!best_matches)
     best_matches=DEFbest_matches;
  fuzzstr.text=malloc(fuzzstr.buflen=BUFSTEP);
  ;{ int i;
     best=malloc(best_matches--*sizeof*best);i=best_matches;
     do
      { best[i]=malloc(sizeof**best);best[i]->hard=malloc(1);
	best[i]->fuzz=malloc(1);best[i]->metric= -SCALE_WEIGHT;
      }
     while(i--);
   }
  for(lastfrom= -1;dodomain||readstr(stdin,&fuzzstr,0);)
   { int meter;long linentry;off_t offs1,offs2;
     unsigned hfile;
     ;{ char*chp;
	static const char tpunctuation[]="@\\/!#$%^&*-_=+|~`';:,.?{}";
#define punctuation	(tpunctuation+3)
	static const char colonpunct[]="/:@!";
#define routepunct	(colonpunct+1)
	chp=fuzzstr.text;
	if(!dodomain)			    /* still have to do with domain? */
	 { char*echp=strchr(chp,'\0')-1;
	   while(*chp&&strchr(punctuation,*chp))
	      chp++;				/* strip leading punctuation */
	   if(*chp=='"'&&!strchr(chp+1,'"'))   /* strip leading unbalanced " */
	      chp++;
	   while(*chp&&strchr(punctuation,*chp))
	      chp++;				/* strip leading punctuation */
	   ;{ const char*colon;				/* no decnet address */
	      if(*(colon=chp+strcspn(chp,colonpunct))==':'&&colon[1]!=':')
		 chp=(char*)colon+1;	       /* strip leading ...: garbage */
	    }
	   while(echp>=chp&&strchr(tpunctuation,*echp))
	      *echp--='\0';		       /* strip trailing punctuation */
	   if(echp>=chp&&*echp=='"'&&strchr(chp,'"')==echp)
	      *echp--='\0';		      /* strip trailing unbalanced " */
	   while(echp>=chp&&strchr(tpunctuation,*echp))
	      *echp--='\0';		       /* strip trailing punctuation */
	   if(echp<chp)
	      continue;
	   if(*chp=='/'&&chp[1]=='/'||			/* eek, it's an URL! */
	      lastfrom<=0&&   /* roughly check if it could be a mail address */
	      !strpbrk(chp,"@/")&&		 /* RFC-822 or X-400 address */
	      (!strchr(chp,'!')||		   /* UUCP bang path address */
	       strchr(chp,'|')||		  /* impossible in addresses */
	       strchr(chp,',')||	  /* only in a source routed address */
	       strstr(chp,".."))&&
	      !(*chp=='<'&&*echp=='>')&&	  /* RFC-822 machine literal */
	      !(incomplete&&strchr(chp,'.')))		      /* domain name */
reject:	    { if(lastfrom<0)
		 lastfrom=!strcmp(SHFROM,chp);
	      continue;			 /* apparently not an e-mail address */
	    }
	   ;{ const char*colon;
	      if((*chp=='@'||*chp=='<'&&chp[1]=='@')&&	 /* leading at's are */
		 (!(colon=strchr(chp,':'))||strchr(routepunct,colon[1])))
		 goto reject;		  /* only allowed on route addresses */
	    }
	   lastfrom=0;tmemmove(fuzzstr.text,chp,echp-chp+2);
	   checkparens('(',')',fuzzstr.text,echp);
	   checkparens('[',']',fuzzstr.text,strchr(fuzzstr.text,'\0'));
	   if(*(chp=fuzzstr.text)=='<'&&*(echp=strchr(chp,'\0')-1)=='>')
	    { if(chp=strstr(chp,">,<"))		/* take the first of a dense */
		 (echp=chp)[1]='\0';			/* list of addresses */
	      if(!strchr(chp=fuzzstr.text,','))	      /* strip '<' and '>' ? */
		 *echp='\0',tmemmove(chp,chp+1,echp-chp);
	    }
	 }
	;{ size_t len;
	   if(!(len=strlen(chp)))		    /* still something left? */
	      continue;			      /* it's gone, next word please */
	   if(dodomain)		   /* add default local domain and reiterate */
	    { dodomain=0;fuzzstr.text=chp=realloc(chp,len+lldomain+1);
	      chp[len]='@';strcpy(chp+len+1,ldomain);len+=lldomain;
	    }
	   else if(ldomain&&!strpbrk(chp,"@!/"))      /* no domain attached? */
	      dodomain=1;			 /* mark it for the next run */
	   fuzzstr.textlen=len;
	 }
	lowcase(&fuzzstr);			   /* cast it into lowercase */
	if(excstr.text&&matchgram(&fuzzstr,&excstr)>=EXCL_THRESHOLD||
	 exc2str.text&&matchgram(&fuzzstr,&exc2str)>=EXCL_THRESHOLD)
	 { free(fuzzstr.itext);
	   continue;
	 }
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
     for(hfile=0;hfile<argc;)
      { int maxmetric=best[best_matches]->metric;
	fseek(hardfile=hfiles[hfile++],(off_t)0,SEEK_SET);ctellinit();
	for(remov_delim=offs2=linentry=0;
	 offs1=offs2,readstr(hardfile,&hardstr,1);)
	 { offs2=ctell(hardfile);linentry++;
	   if(*hardstr.text=='(')
	      continue;				   /* unsuitable for matches */
	   lowcase(&hardstr);meter=matchgram(&fuzzstr,&hardstr);
	   free(hardstr.itext);			 /* check if we had any luck */
	   if(meter>maxmetric&&(fremov||remov_delim||!renam&&!remov))
	    { size_t hardlen;
	      curmatch->metric=maxmetric=meter;curmatch->lentry=linentry;
	      curmatch->offs1=offs1;curmatch->offs2=offs2;
	      curmatch->hardfile=hardfile;
	      free(curmatch->hard);hardlen=hardstr.textlen+1;
	      curmatch->hard=malloc(hardlen+=strlen(hardstr.text+hardlen)+1);
	      tmemmove(curmatch->hard,hardstr.text,hardlen);
	      if(multiple)
	       { struct match*mp,**mmp;
		 free((mp= *(mmp=best+best_matches))->fuzz);free(mp->hard);
		 mp->fuzz=tstrdup(curmatch->fuzz);
		 tmemmove(mp->hard=malloc(hardlen),hardstr.text,hardlen);
		 mp->metric=meter;mp->lentry=linentry;mp->offs1=offs1;
		 mp->offs2=offs2;mp->hardfile=hardfile;
		 ;{ struct match*mpt;
		    while(--mmp>=best&&(mpt= *mmp)->metric<meter)
		       mmp[1]=mpt;			   /* keep it sorted */
		  }
		 mmp[1]=mp;maxmetric=best[best_matches]->metric;
	       }
	    }
	 }
      }
     free(fuzzstr.itext);	 /* maybe this match can be put in the array */
     if(!multiple&&curmatch->metric>-SCALE_WEIGHT) /* of best matches so far */
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
#if 0			 /* metoo support removed, not supported by sendmail */
	if(chkmetoo)
	   printf("%s\n",strcmp(mp->hard+strlen(mp->hard)+1,NOT_METOO)
	    ?metoo_SENDMAIL:nometoo_SENDMAIL);
	else
#endif
	printf("%3ld %-34s %5d %s\n",
	 (long)(charoffs?mp->offs1:mp->lentry),mp->hard,mp->metric,mp->fuzz);
     if((mp= *best)->metric>=minweight)
      { struct match*worse;
	if(renam)
	 { long line;int w1;unsigned maxweight;
	   maxweight=SCALE_WEIGHT/(maxgram?maxgram:1)>>1;;
	   for(i=1,line=mp->lentry,w1=mp->metric,worse=0;
	    i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	      if(mp->lentry==line&&mp->metric+maxweight<w1)
		 goto remv1;
	   for(i=1;i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	      if(mp->metric+maxweight<w1)
remv1:	       { worse=mp;mp= *best;
		 goto remv;
	       }
	   nlog("Couldn't find a proper address pair\n");
	   goto norenam;
	 }
	if(remov)
remv:	 { char*buf;off_t offs1,offs2;size_t readin;
	   buf=malloc(COPYBUF);offs1=mp->offs1;offs2=mp->offs2;
	   hardfile=mp->hardfile;
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
	   fflush(hardfile);			 /* flush before we truncate */
	   if(ftruncate(fileno(hardfile),ftell(hardfile)))
	      do putc('\n',hardfile);	  /* truncate failed, erase the tail */
	      while(ftell(hardfile)<offs2);			  /* by hand */
	 }
	return EXIT_SUCCESS;
      }
   }
  if(remov||renam)
     nlog("Couldn't even find a single address\n");
norenam:
  return 1;
}
