/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1999, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: exopen.c,v 1.37 1999/11/16 06:32:55 guenther Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "robust.h"
#include "misc.h"
#include "common.h"
#include "exopen.h"
#include "lastdirsep.h"

int unique(full,p,len,mode,verbos,chownit)char*const full;char*p;
 const size_t len;const mode_t mode;const int verbos,chownit;
{ static const char s2c[]=".,+:%@";static int serial=STRLEN(s2c);
  static time_t t;char*dot,*end=full+len,*op,*ldp;struct stat filebuf;
  int nicediff,i,didnice,retry=RETRYunique;
  if(chownit&doCHOWN)		  /* semi-critical, try raising the priority */
   { nicediff=nice(0);SETerrno(0);nicediff-=nice(-NICE_RANGE);
     didnice=errno;
   }
  *(end=len?full+len-1:p+UNIQnamelen-1)='\0';
  *(op=p)=UNIQ_PREFIX;dot=ultoan((long)thepid,p+1);
  if(serial<STRLEN(s2c))
     goto in;
  do
   { if(serial>STRLEN(s2c)-1)			     /* roll over the count? */
      { ;{ time_t t2;
	   while(t==(t2=time((time_t*)0)))	/* make sure time has passed */
	      ssleep(1);				   /* tap tap tap... */
	   serial=0;t=t2;
	 }
in:	p=ultoan((long)t,dot+1);
	*p++='.';
	strncpy(p,hostname(),end-p);
      }
     *dot=s2c[serial++];
     i=lstat(full,&filebuf);
#ifdef ENAMETOOLONG
     if(i&&errno==ENAMETOOLONG)
      { if(*op)			      /* first time: where's the lastdirsep? */
	 { if(op!=full&&!strchr(dirsep,op[-1]))
	      op=lastdirsep(full);
	   ldp=op;		   /* keep track to avoid shortening past it */
	   if((op+=MINnamelen+1)>end)		 /* a guess at a safe length */
	      op=end;
	 }
	do
	   *--op='\0';					     /* try chopping */
	while((i=lstat(full,&filebuf))&&errno==ENAMETOOLONG&&op>ldp);
      }	      /* either it stopped being a problem or we ran out of filename */
#endif
   }
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
  while((!i||errno!=ENOENT||	      /* casually check if it already exists */
	 (0>(i=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST))&&
	(i= -1,retry--));
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
ret0:	return chownit&doFD?-1:0;
      }
   }
  if(chownit&doLOCK)
     rwrite(i,"0",1);			   /* pid 0, `works' across networks */
  if(chownit&doFD)
     return i;
  rclose(i);
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
