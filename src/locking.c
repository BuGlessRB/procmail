/************************************************************************
 *	Whatever is needed for (un)locking files in various ways	*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static char rcsid[]="$Id: locking.c,v 1.3 1992/10/02 14:40:27 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "exopen.h"
#include "locking.h"

void lockit(name,lockp)char*name;char**const lockp;
{ int i,permanent=nfsTRY,triedforce=0;struct stat stbuf;time_t t;
  if(*lockp)
   { if(!strcmp(name,*lockp))	/* compare the previous lockfile to this one */
	return;			 /* they're equal, save yourself some effort */
     unlock(lockp);		       /* unlock any previous lockfile FIRST */
   }				  /* to prevent deadlocks (I hate deadlocks) */
  if(!*name)
     return;
  name=tstrdup(name); /* allocate now, so we won't hang on memory *and* lock */
  for(lcking|=lck_LOCKFILE;;)
   { yell("Locking",name);		/* in order to cater for clock skew: */
     if(!xcreat(name,LOCKperm,&t,(int*)0))     /* get time t from filesystem */
      { *lockp=name;break;			   /* lock acquired, hurray! */
      }
     switch(errno)
      { case EEXIST:		   /* check if it's time for a lock override */
	   if(!lstat(name,&stbuf)&&stbuf.st_size<=MAX_LOCK_SIZE&&locktimeout
	    &&!lstat(name,&stbuf)&&locktimeout<t-stbuf.st_mtime)
	     /*
	      * stat() till unlink() should be atomic, but can't guarantee that
	      */
	    { if(triedforce)			/* already tried, not trying */
		 goto faillock;					    /* again */
	      if(S_ISDIR(stbuf.st_mode)||unlink(name))
		 triedforce=1,nlog("Forced unlock denied on"),logqnl(name);
	      else
	       { nlog("Forcing lock on");logqnl(name);suspend();goto ce;
	       }
	    }
	   else
	      triedforce=0;		 /* legitimate iteration, clear flag */
	   break;
	case ENOENT:case ENOTDIR:case EIO:case EACCES:
	   if(--permanent)
	      goto ds;
	   goto faillock;
#ifdef ENAMETOOLONG
	case ENAMETOOLONG:     /* maybe filename too long, shorten and retry */
	   if(0<(i=strlen(name)-1)&&!strchr(dirsep,name[i-1]))
	    { nlog("Truncating");logqnl(name);elog(" and retrying lock\n");
	      name[i]='\0';permanent=nfsTRY;goto ce;
	    }
#endif
	default:
faillock:  nlog("Lock failure on");logqnl(name);goto term;
	case ENOSPC:;
#ifdef EDQUOT
	case EDQUOT:;
#endif
      }
     permanent=nfsTRY;
ds:  sleep((unsigned)locksleep);
ce:  if(nextexit)
term: { free(name);break;		     /* drop the preallocated buffer */
      }
   }
  lcking&=~lck_LOCKFILE;
  if(nextexit)
   { nlog(whilstwfor);elog("lockfile");logqnl(name);terminate();
   }
}

void lcllock()					    /* lock a local lockfile */
{ char*lckfile;
  if(!strcmp(lckfile=tolock?tolock:strcat(buf2,tgetenv(lockext)),
   tgetenv(lockfile)))
     nlog("Deadlock attempted on"),logqnl(lckfile);
  else
     lockit(lckfile,&loclock);
}

void unlock(lockp)char**const lockp;
{ lcking|=lck_LOCKFILE;
  if(*lockp)
   { yell("Unlocking",*lockp);
     if(unlink(*lockp))
	nlog("Couldn't unlock"),logqnl(*lockp);
     if(!nextexit)			   /* if not inside a signal handler */
	free(*lockp);
     *lockp=0;
   }
  lcking&=~lck_LOCKFILE;
  if(nextexit==1)	    /* make sure we are not inside terminate already */
     elog(newline),terminate();
}
					/* an NFS secure exclusive file open */
xcreat(name,mode,tim,chowned)const char*const name;const mode_t mode;
 time_t*const tim;int*const chowned;
{ char*p;int j= -2,i;
  i=lastdirsep(name)-name;strncpy(p=malloc(i+UNIQnamelen),name,i);
  if(unique(p,p+i,mode))	       /* try and rename the unique filename */
   { if(chowned)
	*chowned=chown(p,uid,sgid);			 /* try and chown it */
     if(tim)
      { struct stat stbuf;	 /* return the filesystem time to the caller */
	stat(p,&stbuf);*tim=stbuf.st_mtime;
      }
     j=myrename(p,name);
   }
  free(p);return j;
}

#ifndef fdlock
#ifdef USEflock
#ifndef SYS_FILE_H_MISSING
#include <sys/file.h>
#endif
#endif /* USEflock */
static oldfdlock= -1;				    /* the fd we locked last */
#ifndef NOfcntl_lock
static struct flock flck;		/* why can't it be a local variable? */
#endif
#ifdef USElockf
static long oldlockoffset;
#endif
	/* if you've ever wondered what conditional compilation was good for */
fdlock(fd)						/* watch closely :-) */
{ int ret;
  for(oldfdlock=fd;;
   nlog("Reiterating kernel-lock\n"),sleep((unsigned)locksleep))
   {
#ifndef NOfcntl_lock
     flck.l_type=F_WRLCK;flck.l_whence=SEEK_SET;flck.l_len=0;
     flck.l_start=tell(fd);
#endif
#ifdef USElockf
     oldlockoffset=tell(fd);
#endif
     lcking|=lck_KERNEL;
#ifndef NOfcntl_lock
     ret=fcntl(fd,F_SETLKW,&flck);
#ifdef USElockf
     if(ret|=lockf(fd,F_TLOCK,0L))
ufcntl:
      { flck.l_type=F_UNLCK;fcntl(oldfdlock,F_SETLK,&flck);continue;
      }
#ifdef USEflock
     if(ret|=flock(fd,LOCK_EX|LOCK_NB))
      { lockf(fd,F_ULOCK,0L);goto ufcntl;
      }
#endif /* USEflock */
#endif /* USElockf */
#ifdef USEflock
     if(ret|=flock(fd,LOCK_EX|LOCK_NB))
      { flck.l_type=F_UNLCK;fcntl(oldfdlock,F_SETLK,&flck);continue;
      }
#endif /* USEflock */
#else /* NOfcntl_lock */
#ifdef USElockf
     ret=lockf(fd,F_LOCK,0L);
#ifdef USEflock
     if(ret|=flock(fd,LOCK_EX|LOCK_NB))
      { lockf(fd,F_ULOCK,0L);continue;
      }
#endif /* USEflock */
#else /* USElockf */
#ifdef USEflock
     ret=flock(fd,LOCK_EX);
#endif /* USEflock */
#endif /* USElockf */
#endif /* NOfcntl_lock */
     lcking&=~lck_KERNEL;return ret;
   }
}

fdunlock()
{ int i;
  if(oldfdlock<0)
     return -1;
  i=0;
#ifdef USEflock
  i|=flock(oldfdlock,LOCK_UN);
#endif
#ifdef USElockf
  lseek(oldfdlock,oldlockoffset,SEEK_SET);i|=lockf(oldfdlock,F_ULOCK,0L);
#endif
#ifndef NOfcntl_lock
  flck.l_type=F_UNLCK;i|=fcntl(oldfdlock,F_SETLK,&flck);
#endif
  oldfdlock= -1;return i;
}
#endif /* fdlock */
