/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1999, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: exopen.c,v 1.33 1999/04/22 05:07:05 guenther Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "robust.h"
#include "misc.h"
#include "common.h"
#include "exopen.h"

int unique(full,p,len,mode,verbos,chownit)char*const full;char*p;
 const size_t len;const mode_t mode;const int verbos,chownit;
{ static int serial;static time_t t;char*dot,*cutoff,*end=full+len;
  struct stat filebuf;int nicediff,didnice=0,fd;int retry=RETRYunique;
  const char s2c[]=".,+=";
  if(chownit&doCHOWN)		  /* semi-critical, try raising the priority */
   { nicediff=nice(0);SETerrno(0);nicediff-=nice(-NICE_RANGE);
     if(!errno)
	didnice=1;
   }
  *(end=len?full+len-1:p+UNIQnamelen-1)='\0';
  cutoff=p+MINnamelen;
  *p=UNIQ_PREFIX;dot=ultoan((long)thepid,p+1);
  do
   { if(serial>3)				     /* roll over the count? */
      { time_t t2;
	while(t==(t2=time((time_t*)0)))		/* make sure time has passed */
	   ssleep(2);					   /* tap tap tap... */
	p=ultoan((long)(t=t2),dot+1);
	*p++='.';
	strncpy(p,hostname(),end-p);
	serial=0;
      }
     *dot=s2c[serial++];
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
     if((fd=lstat(full,&filebuf))&&errno==ENAMETOOLONG)
      { *cutoff='\0';
	fd=lstat(full,&filebuf);
      }
   }
  while((!fd||errno==ENOENT&&	      /* casually check if it already exists */
	 (0>(fd=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST))&&
	(fd= -1,retry--));
  if(didnice)
     nice(nicediff);		   /* put back the priority to the old level */
  if(fd<0)
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
     fstat(fd,&fdbuf);			/* match between the file descriptor */
#define NEQ(what)	(fdbuf.what!=filebuf.what)	    /* and the file? */
     if(lstat(full,&filebuf)||filebuf.st_nlink!=1||filebuf.st_size||
	NEQ(st_dev)||NEQ(st_ino)||NEQ(st_uid)||NEQ(st_gid)||
	 chownit&doCHOWN&&
#endif
	 chown(full,uid,sgid))
      { rclose(fd);unlink(full);		 /* forget it, no permission */
ret0:	return chownit&doFD?-1:0;
      }
   }
  if(chownit&doLOCK)
     rwrite(fd,"0",1);			   /* pid 0, `works' across networks */
  if(doFD)
     return fd;
  rclose(fd);
  return 1;
}
				     /* rename MUST fail if already existent */
int myrename(old,newn)const char*const old,*const newn;
{ int fd,serrno;
  fd=hlink(old,newn);serrno=errno;unlink(old);
  if(fd>0)rclose(fd-1);
  SETerrno(serrno);
  return fd<0?-1:0;
}

						     /* NFS-resistant link() */
int rlink(old,newn,st)const char*const old,*const newn;struct stat*st;
{ if(link(old,newn))
   { register int serrno,ret;struct stat sto,stn;
     serrno=errno;ret= -1;
#undef NEQ			       /* compare files to see if the link() */
#define NEQ(what)	(sto.what!=stn.what)	       /* actually succeeded */
     if(lstat(old,&sto)||(ret=1,lstat(newn,&stn)||
	NEQ(st_dev)||NEQ(st_ino)||NEQ(st_uid)||NEQ(st_gid)||
	S_ISLNK(sto.st_mode)))			    /* symlinks are also bad */
      { SETerrno(serrno);
	if(st&&ret>0)
	 { *st=sto;				       /* save the stat data */
	   return ret;				    /* it was a real failure */
	 }
	return -1;
      }
     /*SETerrno(serrno);*/   /* we really succeeded, don't bother with errno */
   }
  return 0;
}
		 /* hardlink with fallback for systems that don't support it */
int hlink(old,newn)const char*const old,*const newn;
{ int ret;struct stat stbuf;
  if(0<(ret=rlink(old,newn,&stbuf)))		      /* try a real hardlink */
   { int fd;
#ifdef O_CREAT				       /* failure due to filesystem? */
     if(stbuf.st_nlink<2&&errno==EXDEV&&     /* try it by conventional means */
	0<=(fd=ropen(newn,O_WRONLY|O_CREAT|O_EXCL,stbuf.st_mode)))
	return fd+1;
#endif
     return -1;
   }
  return ret;				 /* success, or the stat failed also */
}
