/************************************************************************
 *	Miscellaneous routines used by procmail				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: misc.c,v 1.19 1993/02/10 17:08:03 berg Exp $";
#endif
#include "procmail.h"
#include "sublib.h"
#include "robust.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "cstdio.h"
#include "exopen.h"
#include "regexp.h"
#include "goodies.h"
#include "locking.h"
#include "mailfold.h"

struct varval strenvvar[]={{"LOCKSLEEP",DEFlocksleep},
 {"LOCKTIMEOUT",DEFlocktimeout},{"SUSPEND",DEFsuspend},
 {"NORESRETRY",DEFnoresretry},{"TIMEOUT",DEFtimeout},{"VERBOSE",DEFverbose}};
int didchd;
static fakedelivery;
		       /* line buffered to keep concurrent entries untangled */
void elog(newt)const char*const newt;
{ int lnew,i;static lold;static char*old;char*p;
#ifndef O_CREAT
  lseek(STDERR,0L,SEEK_END);		  /* locking should be done actually */
#endif
  if(!(lnew=strlen(newt))||nextexit)			     /* force flush? */
     goto flush;
  i=lold+lnew;
  if(p=lold?realloc(old,i):malloc(i))			 /* unshelled malloc */
   { memmove((old=p)+lold,newt,(size_t)lnew);			   /* append */
     if(p[(lold=i)-1]=='\n')					     /* EOL? */
	rwrite(STDERR,p,i),lold=0,free(p);		/* flush the line(s) */
   }
  else						   /* no memory, force flush */
flush:
   { if(lold)
      { rwrite(STDERR,old,lold);lold=0;
	if(!nextexit)
	   free(old);			/* don't use free in signal handlers */
      }
     if(lnew)
	rwrite(STDERR,newt,lnew);
   }
}

#include "shell.h"

void ignoreterm P((void))
{ signal(SIGTERM,SIG_IGN);signal(SIGHUP,SIG_IGN);signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
}

void writeerr(line)const char*const line;
{ nlog("Error while writing to");logqnl(line);
}

forkerr(pid,a)const pid_t pid;const char*const a;
{ if(pid==-1)
   { nlog("Failed forking");logqnl(a);return 1;
   }
  return 0;
}

void progerr(line)const char*const line;
{ nlog("Program failure of");logqnl(line);
}

void chderr(dir)const char*const dir;
{ nlog("Couldn't chdir to");logqnl(dir);
}

void readerr(file)const char*const file;
{ nlog("Couldn't read");logqnl(file);
}

void yell(a,b)const char*const a,*const b;		/* log if VERBOSE=on */
{ if(verbose)
     nlog(a),logqnl(b);
}

void nlog(a)const char*const a;
{ elog(procmailn);elog(": ");elog(a);
}

void logqnl(a)const char*const a;
{ elog(oquote);elog(a);elog(cquote);
}

void skipped(x)const char*const x;
{ if(*x)
     nlog("Skipped"),logqnl(x);
}

nextrcfile P((void))		/* next rcfile specified on the command line */
{ const char*p;
  while(p= *gargv)
   { gargv++;
     if(!strchr(p,'='))
      { rcfile=p;return 1;
      }
   }
  return 0;
}

void sterminate P((void))
{ static const char*const msg[]={"memory","fork",	  /* crosscheck with */
   "a file descriptor","a kernel-lock"};	  /* lck_ defs in procmail.h */
  ignoreterm();
  if(pidchild>0)	    /* don't kill what is not ours, we might be root */
     kill(pidchild,SIGTERM);
  if(!nextexit)
   { nextexit=1;nlog("Terminating prematurely");
     if(!(lcking&lck_LOCKFILE))
      { register unsigned i,j;
	if(i=(lcking&~(lck_ALLOCLIB|lck_LOCKFILE))>>1)
	 { elog(whilstwfor);
	   for(j=0;!((i>>=1)&1);j++);
	   elog(msg[j]);
	 }
	elog(newline);terminate();
      }
   }
}

void terminate P((void))
{ ignoreterm();
  if(retvl2!=EX_OK)
     fakedelivery=0,retval=retvl2;
  if(getpid()==thepid)
   { if(retval!=EX_OK)
      { tofile=0;lasttell= -1;		 /* make sure that logabstract knows */
	lastfolder=fakedelivery?"**Lost**":		/* don't free() here */
	 retval==EX_TEMPFAIL?"**Requeued**":"**Bounced**";
      }
     logabstract();closerc();
     if(!(lcking&lck_ALLOCLIB))			/* don't reenter malloc/free */
	exectrap(tgetenv("TRAP"));
     nextexit=2;unlock(&loclock);unlock(&globlock);fdunlock();
   }
  exit(fakedelivery==2?EX_OK:retval);
}

void suspend P((void))
{ long t;
  sleep((unsigned)suspendv);
  if(alrmtime)
     if((t=alrmtime-time((time_t*)0))<=1)	  /* if less than 1s timeout */
	ftimeout();				  /* activate it by hand now */
     else		    /* set it manually again, to avoid problems with */
	alarm((unsigned)t);	/* badly implemented sleep library functions */
}

void app_val(sp,val)struct dyna_long*const sp;const long val;
{ if(sp->filled==sp->tspace)			    /* growth limit reached? */
   { if(!sp->offs)
	sp->offs=malloc(1);
     sp->offs=realloc(sp->offs,(sp->tspace+=4)*sizeof sp->offs);   /* expand */
   }
  sp->offs[sp->filled++]=val;				     /* append to it */
}

alphanum(c)const unsigned c;
{ return c-'0'<='9'-'0'||c-'a'<='z'-'a'||c-'A'<='Z'-'A'||c=='_';
}

void firstchd P((void))
{ if(!didchd)				       /* have we been here already? */
   { const char*p;
     didchd=1;			      /* no, well, then try an initial chdir */
     if(chdir(p=tgetenv(maildir)))
      { chderr(p);
	if(chdir(p=tgetenv(home)))
	   chderr(p);
      }
   }
}

void srequeue P((void))
{ retval=EX_TEMPFAIL;sterminate();
}

void slose P((void))
{ fakedelivery=2;sterminate();
}

void sbounce P((void))
{ retval=EX_CANTCREAT;sterminate();
}

void catlim(src)register const char*src;
{ register char*dest=buf;register size_t lim=linebuf;
  while(lim&&*dest)
     dest++,lim--;
  if(lim)
   { while(--lim&&(*dest++= *src++));
     *dest='\0';
   }
}

void setdef(name,contents)const char*const name,*const contents;
{ strcat(strcat(strcpy((char*)(sgetcp=buf2),name),"="),contents);
  readparse(buf,sgetc,2);sputenv(buf);
}

void metaparse(p)const char*p;				    /* result in buf */
{ if(sh=!!strpbrk(p,tgetenv(shellmetas)))
     strcpy(buf,p);			 /* copy literally, shell will parse */
  else
#ifndef GOT_bin_test
   { sgetcp=p=tstrdup(p);
     readparse(buf,sgetc,0);				/* parse it yourself */
     if(!strcmp(test,buf))
	strcpy(buf,p),sh=1;			       /* oops, `test' found */
     free((char*)p);
   }
#else
     sgetcp=p,readparse(buf,sgetc,0);
#endif
}

void concatenate(p)register char*p;
{ while(*p!=TMNATE)			  /* concatenate all other arguments */
   { while(*p++);
     p[-1]=' ';
   }
  *p=p[-1]='\0';
}

char*lastdirsep(filename)const char*filename;	 /* finds the next character */
{ const char*p;					/* following the last DIRSEP */
  while(p=strpbrk(filename,dirsep))
     filename=p+1;
  return(char*)filename;
}

char*cat(a,b)const char*const a,*const b;
{ return strcat(strcpy(buf,a),b);
}

char*tstrdup(a)const char*const a;
{ int i;
  i=strlen(a)+1;return tmemmove(malloc(i),a,i);
}

const char*tgetenv(a)const char*const a;
{ const char*b;
  return(b=getenv(a))?b:"";
}

char*cstr(a,b)char*const a;const char*const b;	/* dynamic buffer management */
{ if(a)
     free(a);
  return tstrdup(b);
}

char*skpspace(chp)const char*chp;
{ for(;;chp++)
     switch(*chp)
      { case ' ':case '\t':continue;
	default:return(char*)chp;
      }
}

char*gobenv(chp)char*chp;
{ int found,i;
  for(found=0;alphanum(i=getb());found=1,*chp++=i);
  *chp='\0';ungetb(i);
  switch(i)
   { case ' ':case '\t':case '\n':case '=':return chp;
   }
  return 0;
}

void asenvcpy(src)char*src;
{ strcpy(buf,src);
  if(src=strchr(buf,'='))			     /* is it an assignment? */
   { strcpy((char*)(sgetcp=buf2),++src);readparse(src,sgetc,2);sputenv(buf);
     src[-1]='\0';asenv(src);
   }
}

void asenv(chp)const char*const chp;
{ static const char slinebuf[]="LINEBUF",logfile[]="LOGFILE",Log[]="LOG",
   sdelivered[]="DELIVERED",includerc[]="INCLUDERC",eumask[]="UMASK",
   host[]="HOST";
  if(!strcmp(buf,slinebuf))
   { if((linebuf=renvint(0L,chp)+XTRAlinebuf)<MINlinebuf+XTRAlinebuf)
	linebuf=MINlinebuf+XTRAlinebuf;		       /* check minimum size */
     free(buf);free(buf2);buf=malloc(linebuf);buf2=malloc(linebuf);
   }
  else if(!strcmp(buf,maildir))
     if(chdir(chp))
	chderr(chp);
     else
	didchd=1;
  else if(!strcmp(buf,logfile))
     openlog(chp);
  else if(!strcmp(buf,Log))
     elog(chp);
  else if(!strcmp(buf,sdelivered))			    /* fake delivery */
   { if(renvint(0L,chp))				    /* is it really? */
      { lcking|=lck_LOCKFILE;		    /* just to prevent interruptions */
	if((thepid=sfork())>0)
	 { nextexit=2;lcking&=~lck_LOCKFILE;exit(retvl2);
	 }					/* signals may cause trouble */
	if(!forkerr(thepid,procmailn))
	   fakedelivery=1;
	thepid=getpid();lcking&=~lck_LOCKFILE;
	if(nextexit)				 /* signals occurred so far? */
	   elog(newline),terminate();
      }
   }
  else if(!strcmp(buf,lockfile))
     lockit((char*)chp,&globlock);
  else if(!strcmp(buf,eumask))
     umask((int)strtol(chp,(char**)0,8));
  else if(!strcmp(buf,includerc))
     pushrc(chp);
  else if(!strcmp(buf,host))
   { const char*name;
     if(strncmp(chp,name=hostname(),HOSTNAMElen))
      { yell("HOST mismatched",name);
	if(rc<0||!nextrcfile())			  /* if no rcfile opened yet */
	   retval=EX_OK,terminate();		  /* exit gracefully as well */
	closerc();rc=rc_NOFILE;
      }
   }
  else
   { int i=MAXvarvals;
     do					      /* several numeric assignments */
	if(!strcmp(buf,strenvvar[i].name))
	 { strenvvar[i].val=renvint(strenvvar[i].val,chp);break;
	 }
     while(i--);
   }
}

long renvint(i,env)const long i;const char*const env;
{ const char*p;long t;
  t=strtol(env,(char**)&p,10);			  /* parse like a decimal nr */
  if(p==env)
   { for(;;p++)					  /* skip leading whitespace */
      { switch(*p)
	 { case '\t':case ' ':continue;
	 }
	break;
      }
     t=i;
     if(!strnIcmp(p,"on",(size_t)2)||!strnIcmp(p,"y",(size_t)1)||
      !strnIcmp(p,"t",(size_t)1)||!strnIcmp(p,"e",(size_t)1))
	t=1;
     else if(!strnIcmp(p,"off",(size_t)3)||!strnIcmp(p,"n",(size_t)1)||
      !strnIcmp(p,"f",(size_t)1)||!strnIcmp(p,"d",(size_t)1))
	t=0;
   }
  return t;
}

char*egrepin(expr,source,len,casesens)char*expr;const char*source;
 const long len;
{ source=(const char*)bregexec((struct eps*)(expr=(char*)
   bregcomp(expr,!casesens)),(const uchar*)source,len>0?
   (size_t)len:(size_t)0,!casesens);
  free(expr);return(char*)source;
}
