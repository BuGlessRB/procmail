/************************************************************************
 *	The fault-tolerant system-interface				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: robust.c,v 1.9 1993/01/19 11:55:25 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "misc.h"
#include "mailfold.h"

#define nomemretry	noresretry
#define noforkretry	noresretry
		       /* set nextexit to prevent elog() from using malloc() */
static void nomemerr P((void))
{ nextexit=2;nlog("Out of memory\n");
  if(buf2)
   { buf[linebuf-1]=buf2[linebuf-1]='\0';elog("buffer 0:");logqnl(buf);
     elog("buffer 1:");logqnl(buf2);
   }
  if(retval!=EX_TEMPFAIL)
     retval=EX_OSERR;
  terminate();
}

void*tmalloc(len)const size_t len;    /* this malloc can survive a temporary */
{ void*p;int i;				    /* "out of swap space" condition */
  lcking|=lck_ALLOCLIB;
  if(p=malloc(len))
     goto ret;
  lcking|=lck_MEMORY;
  if(p=malloc(1))
     free(p);			   /* works on some systems with latent free */
  for(i=nomemretry;i<0||i--;)
   { suspend();		     /* problems?  don't panic, wait a few secs till */
     if(p=malloc(len))	     /* some other process has paniced (and died 8-) */
ret:  { lcking&=~(lck_MEMORY|lck_ALLOCLIB);return p;
      }
   }
  nomemerr();
}

void*trealloc(old,len)void*const old;const size_t len;
{ void*p;int i;
  lcking|=lck_ALLOCLIB;
  if(p=realloc(old,len))
     goto ret;				    /* for comment see tmalloc above */
  lcking|=lck_MEMORY;
  if(p=malloc(1))
    free(p);
  for(i=nomemretry;i<0||i--;)
   { suspend();
     if(p=realloc(old,len))
ret:  { lcking&=~(lck_MEMORY|lck_ALLOCLIB);return p;
      }
   }
  nomemerr();
}

void tfree(p)void*const p;
{ lcking|=lck_ALLOCLIB;free(p);lcking&=~lck_ALLOCLIB;
}

#include "shell.h"

pid_t sfork P((void))			/* this fork can survive a temporary */
{ pid_t i;int r;			   /* "process table full" condition */
  elog("");r=noforkretry;			  /* flush log, just in case */
  while((i=fork())==-1)
   { lcking|=lck_FORK;
     if(!(r<0||r--))
	break;
     suspend();
   }
  lcking&=~lck_FORK;return i;
}

void openlog(file)const char*file;
{ int i;
  if(!*file)						   /* empty LOGFILE? */
     file=devnull;				 /* substitute the bitbucket */
  if(0>(i=opena(file)))
     writeerr(file);			      /* error, keep the old LOGFILE */
  else
     rclose(STDERR),rdup(i),rclose(i),logopened=1;
}

opena(a)const char*const a;
{ if(asgnlastf)
     asgnlastf=0,lastfolder=cstr(lastfolder,a);
  yell("Opening",a);
#ifdef O_CREAT
  return ropen(a,O_WRONLY|O_APPEND|O_CREAT,NORMperm);
#else
  ;{ int fd;
     return(fd=ropen(a,O_WRONLY,0))<0?creat(a,NORMperm):fd;
   }
#endif
}

ropen(name,mode,mask)const char*const name;const int mode;const mode_t mask;
{ int i,r;					       /* a SysV secure open */
  for(r=noresretry,lcking|=lck_FILDES;0>(i=open(name,mode,mask));)
     if(errno!=EINTR&&!(errno==ENFILE&&(r<0||r--)))
	break;		 /* survives a temporary "file table full" condition */
  lcking&=~lck_FILDES;return i;
}

rpipe(fd)int fd[2];
{ int i,r;					  /* catch "file table full" */
  for(r=noresretry,lcking|=lck_FILDES;0>(i=pipe(fd));)
     if(!(errno==ENFILE&&(r<0||r--)))
      { *fd=fd[1]= -1;break;
      }
  lcking&=~lck_FILDES;return i;
}

rdup(p)const int p;
{ int i,r;					  /* catch "file table full" */
  for(r=noresretry,lcking|=lck_FILDES;0>(i=dup(p));)
     if(!(errno==ENFILE&&(r<0||r--)))
	break;
  lcking&=~lck_FILDES;return i;
}

rclose(fd)const int fd;		      /* a SysV secure close (signal immune) */
{ int i;
  while((i=close(fd))&&errno==EINTR);
  return i;
}

rread(fd,a,len)const int fd,len;void*const a;	       /* a SysV secure read */
{ int i;
  while(0>(i=read(fd,a,(size_t)len))&&errno==EINTR);
  return i;
}

rwrite(fd,a,len)const int fd,len;const void*const a;  /* a SysV secure write */
{ int i;
  while(0>(i=write(fd,a,(size_t)len))&&errno==EINTR);
  return i;
}
