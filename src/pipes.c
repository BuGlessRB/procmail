/************************************************************************
 *	Routines related to setting up pipes and filters		*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: pipes.c,v 1.12 1993/01/19 11:55:20 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "cstdio.h"
#include "goodies.h"
#include "mailfold.h"

pid_t pidchild;
volatile time_t alrmtime;
static char*lastexec,*backblock;
static long backlen;	       /* length of backblock, filter recovery block */
static pid_t pidfilt;
static pipw,pbackfd[2];			       /* the emergency backpipe :-) */

void inittmout(progname)const char*const progname;
{ lastexec=cstr(lastexec,progname);
  alrmtime=timeoutv?time((time_t*)0)+(unsigned)timeoutv:0;
  alarm((unsigned)timeoutv);
}

void ftimeout P((void))
{ alarm(0);alrmtime=0;
  if(pidchild>0&&!kill(pidchild,SIGTERM))	   /* careful, killing again */
	nlog("Timeout, terminating"),logqnl(lastexec);
  signal(SIGALRM,(void(*)())ftimeout);
}

static void stermchild P((void))
{ if(pidfilt>0)		    /* don't kill what is not ours, we might be root */
     kill(pidfilt,SIGTERM);
  if(!Stdout)
   { nlog("Rescue of unfiltered data ");
     if(dump(PWRB,backblock,backlen))	  /* pump data back via the backpipe */
	elog("failed\n");
     else
	elog("succeeded\n");
   }
  exit(lexitcode);
}

static void childsetup P((void))
{ lexitcode=EX_UNAVAILABLE;signal(SIGTERM,(void(*)())stermchild);
  signal(SIGINT,(void(*)())stermchild);signal(SIGHUP,(void(*)())stermchild);
  signal(SIGQUIT,(void(*)())stermchild);closerc();
}

static void getstdin(pip)const int pip;
{ rclose(STDIN);rdup(pip);rclose(pip);
}

static void callnewprog(newname)const char*const newname;
{ if(sh)					 /* should we start a shell? */
   { const char*newargv[4];
     yell(executing,newname);newargv[3]=0;newargv[2]=newname;
     newargv[1]=tgetenv(shellflags);*newargv=tgetenv(shell);shexec(newargv);
   }
  ;{ register const char*p;int argc;const char**newargv;
     argc=1;p=newname;	     /* If no shell, chop up the arguments ourselves */
     if(verbose)
      { nlog(executing);elog(oquote);goto no_1st_comma;
      }
     do					     /* show chopped up command line */
      { if(verbose)
	 { elog(",");
no_1st_comma:
	   elog(p);
	 }
	while(*p++);
      }
     while(argc++,*p!=TMNATE);
     if(verbose)
	elog(cquote);				      /* allocate argv array */
     newargv=malloc(argc*sizeof*newargv);p=newname;argc=0;
     do
      { newargv[argc++]=p;
	while(*p++);
      }
     while(*p!=TMNATE);
     newargv[argc]=0;shexec(newargv);
   }
}

pipthrough(line,source,len)char*line,*source;const long len;
{ int pinfd[2],poutfd[2];
  if(Stdout)
     PWRB=PRDB= -1;
  else
     rpipe(pbackfd);
  rpipe(pinfd);						 /* main pipes setup */
  if(!(pidchild=sfork()))			/* create a sending procmail */
   { backblock=source;backlen=len;childsetup();rclose(PRDI);rclose(PRDB);
     rpipe(poutfd);rclose(STDOUT);
     if(!(pidfilt=sfork()))				/* create the filter */
      { rclose(PWRB);rclose(PWRO);rdup(PWRI);rclose(PWRI);getstdin(PRDO);
	callnewprog(line);
      }
     rclose(PWRI);rclose(PRDO);
     if(forkerr(pidfilt,line))
	rclose(PWRO),stermchild();
     if(dump(PWRO,source,len)&&!ignwerr)  /* send in the text to be filtered */
	writeerr(line),lexitcode=EX_IOERR,stermchild();
     if(pwait&&waitfor(pidfilt)!=EX_OK)	 /* check the exitcode of the filter */
      { pidfilt=0;
	if(!(pwait&2))				  /* do we put it on report? */
	   progerr(line);
	stermchild();
      }
     rclose(PWRB);exit(EX_OK);			  /* allow parent to proceed */
   }
  rclose(PWRB);rclose(PWRI);getstdin(PRDI);
  if(forkerr(pidchild,procmailn))
     return 1;
  if(Stdout)
   { retStdout(readdyn(Stdout,&Stdfilled));
     if(pwait)
	return pipw;
   }
  return 0;		    /* we stay behind to read back the filtered text */
}

long pipin(line,source,len)char*const line;char*source;long len;
{ int poutfd[2];
  rpipe(poutfd);
  if(!(pidchild=sfork()))				    /* spawn program */
     rclose(PWRO),closerc(),getstdin(PRDO),callnewprog(line);
  rclose(PRDO);
  if(forkerr(pidchild,line))
     return 1;					    /* dump mail in the pipe */
  if((len=dump(PWRO,source,len))&&(!ignwerr||(len=0)))
     writeerr(line);		       /* pipe was shut in our face, get mad */
  if(pwait&&waitfor(pidchild)!=EX_OK)	    /* optionally check the exitcode */
   { if(!(pwait&2))				  /* do we put it on report? */
	progerr(line);
     len=1;
   }
  pidchild=0;
  if(!sh)
     concatenate(line);
  if(asgnlastf)
     asgnlastf=0,lastfolder=cstr(lastfolder,line);
  return len;
}

char*readdyn(bf,filled)char*bf;long*const filled;
{ int i;long oldsize;
  oldsize= *filled;goto jumpin;
  do
   { *filled+=i;				/* change listed buffer size */
jumpin:
#ifdef SMALLHEAP
     if((size_t)*filled>=(size_t)(*filled+BLKSIZ))
	lcking|=lck_MEMORY,nomemerr();
#endif
     bf=realloc(bf,*filled+BLKSIZ);    /* dynamically adjust the buffer size */
jumpback:;
   }
  while(0<(i=rread(STDIN,bf+*filled,BLKSIZ)));			/* read mail */
  if(pidchild>0)
   { if(!Stdout)
      { getstdin(PRDB);			       /* filter ready, get backpipe */
	if(1==rread(STDIN,buf,1))		      /* backup pipe closed? */
	 { bf=realloc(bf,(*filled=oldsize+1)+BLKSIZ);bf[oldsize]= *buf;
	   if(pwait)
	      waitfor(pidchild);
	   pidchild=0;goto jumpback;	       /* filter goofed, rescue data */
	 }
      }
     if(pwait)
	pipw=waitfor(pidchild);		      /* reap your child in any case */
   }
  pidchild=0;					/* child must be gone by now */
  if(!*filled)
     return realloc(bf,1);		     /* +1 for housekeeping purposes */
  return realloc(bf,*filled+1);			/* minimise the buffer space */
}

char*fromprog(name,dest,max)char*name;char*const dest;size_t max;
{ int pinfd[2],poutfd[2];int i;char*p;
  concon('\n');rpipe(pinfd);inittmout(name);
  if(!(pidchild=sfork()))			/* create a sending procmail */
   { Stdout=name;childsetup();rclose(PRDI);rpipe(poutfd);rclose(STDOUT);
     if(!(pidfilt=sfork()))			     /* spawn program/filter */
	rclose(PWRO),rdup(PWRI),rclose(PWRI),getstdin(PRDO),callnewprog(name);
     rclose(PWRI);rclose(PRDO);
     if(forkerr(pidfilt,name))
	rclose(PWRO),stermchild();
     dump(PWRO,themail,filled);waitfor(pidfilt);exit(lexitcode);
   }
  rclose(PWRI);p=dest;
  if(!forkerr(pidchild,name))
   { name=tstrdup(name);
     while(0<(i=rread(PRDI,p,max))&&(p+=i,max-=i));	    /* read its lips */
     if(0<rread(PRDI,p,1))
	nlog("Excessive output quenched from"),logqnl(name);
     rclose(PRDI);free(name);
     while(--p>=dest&&*p=='\n');    /* trailing newlines should be discarded */
     p++;waitfor(pidchild);
   }
  else
     rclose(PRDI);
  pidchild=0;*p='\0';return p;
}

void exectrap(tp)const char*const tp;
{ if(*tp)
   { int newret;
     metaparse(tp);inittmout(buf);
     if(!(pidchild=sfork()))	     /* connect stdout to stderr before exec */
      { signal(SIGTERM,SIG_DFL);signal(SIGINT,SIG_DFL);signal(SIGHUP,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);rclose(STDOUT);rdup(STDERR);callnewprog(buf);
      }
     if(!forkerr(pidchild,buf)&&(newret=waitfor(pidchild))!=EX_OK)
	retval=newret;			       /* supersede the return value */
   }
}
