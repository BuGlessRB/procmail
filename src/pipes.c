/************************************************************************
 *	Routines related to setting up pipes and filters		*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: pipes.c,v 1.15 1993/06/21 14:24:46 berg Exp $";
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
   { static const char rescdata[]="Rescue of unfiltered data ";
     if(dump(PWRB,backblock,backlen))	  /* pump data back via the backpipe */
	nlog(rescdata),elog("failed\n");
     else if(pwait!=4)			/* are we not looking the other way? */
	nlog(rescdata),elog("succeeded\n");
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
  ;{ register const char*p;int argc;
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
	if(verbose&&p-1==All_args&&crestarg)		  /* any "$@" found? */
	 { const char*const*walkargs=restargv;
	   goto No_1st_comma;
	   do
	    { elog(",");
No_1st_comma: elog(*walkargs);					/* expand it */
	    }
	   while(*++walkargs);
	 }
	if(p-1==All_args)
	   argc+=crestarg-1;			       /* and account for it */
      }
     while(argc++,p!=Tmnate);
     if(verbose)
	elog(cquote);				      /* allocate argv array */
     ;{ const char**newargv;
	newargv=malloc(argc*sizeof*newargv);p=newname;argc=0;
	do
	 { newargv[argc++]=p;
	   while(*p++);
	   if(p-1==All_args&&crestarg)
	    { const char*const*walkargs=restargv;	      /* expand "$@" */
	      argc--;
	      while(newargv[argc]= *walkargs++)
		 argc++;
	    }
	 }
	while(p!=Tmnate);
	newargv[argc]=0;shexec(newargv);
      }
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
	if(pwait&2)				  /* do we put it on report? */
	   pwait=4;			     /* no, we'll look the other way */
	else
	   progerr(line);		      /* I'm going to tell my mommy! */
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
  setlastfolder(line);return len;
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
     metaparse(tp);concon('\n');inittmout(buf);
     if(!(pidchild=sfork()))	     /* connect stdout to stderr before exec */
      { int poutfd[2];
	Stdout=buf;childsetup();rpipe(poutfd);rclose(STDOUT);pidfilt=thepid;
	getstdin(PRDO);
	if(!(pidchild=sfork()))			/* fork off sending procmail */
	 { rclose(STDIN);rclose(STDERR);dump(PWRO,themail,filled);
	   exit(lexitcode);		/* finished dumping to stdin of TRAP */
	 }					 /* call up the TRAP program */
	rclose(PWRO);rdup(STDERR);forkerr(pidchild,buf);callnewprog(buf);
      }
     if(!forkerr(pidchild,buf)&&(newret=waitfor(pidchild))!=EX_OK)
	retval=newret;			       /* supersede the return value */
   }
}
