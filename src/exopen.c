/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: exopen.c,v 1.15 1994/04/05 15:34:20 berg Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "robust.h"
#include "misc.h"
#include "common.h"
#include "exopen.h"

unique(full,p,mode,verbos,chownit)const char*const full;char*p;
 const mode_t mode;const int verbos,chownit;
{ unsigned long retry=mrotbSERIAL;int i;struct stat filebuf;
  int nicediff,didnice=0;
  if(chownit)			  /* semi-critical, try raising the priority */
   { nicediff=nice(0);errno=0;nicediff-=nice(-NICE_RANGE);
     if(errno!=EPERM)
	didnice=1;
   }
  do						  /* create unique file name */
   { *p=UNIQ_PREFIX;p=ultoan(maskSERIAL&(retry-=irotbSERIAL)+(long)thepid,p+1);
     strncpy(p,hostname(),HOSTNAMElen);p[HOSTNAMElen]='\0';
   }
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
  while((chownit&&!lstat(full,&filebuf))||
	(0>(i=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST)&&
	retry);	    /* casually check if it already exists (highly unlikely) */
  if(didnice)
     nice(nicediff);		   /* put back the priority to the old level */
  if(i<0)
   { if(verbos)			      /* this error message can be confusing */
	writeerr(full);					 /* for casual users */
     goto ret0;
   }
  if(chownit)
#ifdef NOfstat
   { rclose(i);
     if(
#else
   { struct stat fdbuf;
     fstat(i,&fdbuf);rclose(i);		/* match between the file descriptor */
#define NEQ(what)	(fdbuf.what!=filebuf.what)	    /* and the file? */
     if(lstat(full,&filebuf)||fdbuf.st_nlink!=1||filebuf.st_nlink!=1||
	NEQ(st_ino)||NEQ(st_mode)||NEQ(st_uid)||NEQ(st_gid)||
#endif
	chown(full,uid,sgid))
      { unlink(full);				 /* forget it, no permission */
ret0:	return 0;
      }
   }
  else
     rclose(i);
  return 1;
}
				     /* rename MUST fail if already existent */
myrename(old,newn)const char*const old,*const newn;
{ int i,serrno;
  i=hlink(old,newn);serrno=errno;unlink(old);errno=serrno;return i;
}
		 /* hardlink with fallback for systems that don't support it */
hlink(old,newn)const char*const old,*const newn;
{ if(link(old,newn))				      /* try a real hardlink */
   { int i,serrno;struct stat stbuf;
     serrno=errno;i=lstat(old,&stbuf);errno=serrno;
     if(i&&S_ISLNK(stbuf.st_mode))		/* no stat or symbolic link? */
	goto retfail;				     /* yuk, don't accept it */
     if(stbuf.st_nlink!=2)
      { if(serrno!=EXDEV)		       /* failure due to filesystem? */
	   goto retfail;		     /* try it by conventional means */
#ifdef O_CREAT
	if(0>(i=ropen(newn,O_WRONLY|O_CREAT|O_EXCL,stbuf.st_mode)))
#endif
retfail:   return -1;
	rclose(i);
      }
   }
  return 0;
}
