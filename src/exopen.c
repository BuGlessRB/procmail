/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1999, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: exopen.c,v 1.30 1999/02/21 04:13:27 guenther Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "robust.h"
#include "misc.h"
#include "common.h"
#include "exopen.h"

int unique(full,p,mode,verbos,chownit)const char*const full;char*p;
 const mode_t mode;const int verbos,chownit;
{ unsigned long retry=mrotbSERIAL;int i;struct stat filebuf;
  int nicediff,didnice=0;char*orig;
  if(chownit&doCHOWN)		  /* semi-critical, try raising the priority */
   { nicediff=nice(0);SETerrno(0);nicediff-=nice(-NICE_RANGE);
     if(!errno)
	didnice=1;
   }
  *p=UNIQ_PREFIX;orig=p+1;
  do						  /* create unique file name */
   { p=ultoan(maskSERIAL&(retry-=irotbSERIAL)+(long)thepid,orig);
     *p++='.';
     strncpy(p,hostname(),HOSTNAMElen);p[HOSTNAMElen]='\0';
   }
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
  while(!lstat(full,&filebuf)||
	(0>(i=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST)&&
	retry);	    /* casually check if it already exists (highly unlikely) */
  if(didnice)
     nice(nicediff);		   /* put back the priority to the old level */
  if(i<0)
   { if(verbos)			      /* this error message can be confusing */
	writeerr(full);					 /* for casual users */
     goto ret0;
   }
#ifdef NOfstat
  if(chownit&doCHOWN)
   { if(
#else
  if(chownit&doCHECK)
   { struct stat fdbuf;
     fstat(i,&fdbuf);			/* match between the file descriptor */
#define NEQ(what)	(fdbuf.what!=filebuf.what)	    /* and the file? */
     if(lstat(full,&filebuf)||filebuf.st_nlink!=1||filebuf.st_size||
	NEQ(st_dev)||NEQ(st_ino)||NEQ(st_uid)||NEQ(st_gid)||
	 chownit&doCHOWN&&
#endif
	 chown(full,uid,sgid))
      { rclose(i);unlink(full);			 /* forget it, no permission */
ret0:	return 0;
      }
   }
  if(chownit&doLOCK)
     rwrite(i,"0",1);			   /* pid 0, `works' across networks */
  rclose(i);
  return 1;
}
				     /* rename MUST fail if already existent */
int myrename(old,newn)const char*const old,*const newn;
{ int i,serrno;
  i=hlink(old,newn);serrno=errno;unlink(old);SETerrno(serrno);
  return i;
}
		 /* hardlink with fallback for systems that don't support it */
int hlink(old,newn)const char*const old,*const newn;
{ if(link(old,newn))				      /* try a real hardlink */
   { int serrno;struct stat stbuf,stbuf2;
     serrno=errno;
     if(lstat(old,&stbuf)||S_ISLNK(stbuf.st_mode))    /* no stat or symlink? */
	goto retfail;				     /* yuk, don't accept it */
#undef NEQ			       /* compare files to see if the link() */
#define NEQ(what)	(stbuf.what!=stbuf2.what)      /* actually succeeded */
     if(stbuf.st_nlink!=2||lstat(newn,&stbuf2)||NEQ(st_dev)||NEQ(st_ino))
      { int fd;
	if(serrno!=EXDEV)		       /* failure due to filesystem? */
	   goto retfail;		     /* try it by conventional means */
#ifdef O_CREAT
	if(0>(fd=ropen(newn,O_WRONLY|O_CREAT|O_EXCL,stbuf.st_mode)))
#endif
retfail: { SETerrno(serrno);
	   return -1;
	 }
	rclose(fd);
      }
     SETerrno(serrno);
   }
  return 0;
}
