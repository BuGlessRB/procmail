/************************************************************************
 *	Memory block routines						*
 *									*
 *	Copyright (c) 1997-1999 Philip Guenther <guenther@gac.edu>	*
 *	This file may be redistributed under the same conditions	*
 *	as the other source files in the procmail suite.		*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: memblk.c,v 1.2 2000/09/28 01:23:30 guenther Exp $"
#endif
#include "procmail.h"
#include "robust.h"
#include "exopen.h"
#include "memblk.h"
#include "shell.h"

#ifdef USE_MMAP
int ISprivate;
#include <sys/mman.h>
#define P_RW		(PROT_READ|PROT_WRITE)
#define MMAP_LEN	(STRLEN(MMAP_DIR)+UNIQnamelen+1)
#define MMAP_PERM	(NORMperm&~INIT_UMASK)
#define set_fd(mb,num)	mb->fd=(num)
#else
#define set_fd(mb,num)	do{}while(0)
#endif

void makeblock(mb,len)memblk*const mb;const long len;
{ mb->len=0;mb->p=malloc(1);set_fd(mb,-1);
  if(len)
     resizeblock(mb,len,0);
}

void freeblock(mb)memblk*const mb;
{
#ifdef USE_MMAP
  if(mb->fd>=0)
   { munmap(mb->p,mb->len+1);
     close(mb->fd);
   }
  else
#endif
    free(mb->p);
}

int resizeblock(mb,len,nonfatal)memblk*const mb;const long len;
 const int nonfatal;
{ if(len==mb->len)
     goto ret1;
  if(!len)
   { freeblock(mb);
     mb->len=0;mb->p=malloc(1);set_fd(mb,-1);
     goto ret1;
   }
#ifdef USE_MMAP
  if(len>MAXinMEM&&mb->fd<0)			      /* time to switch over */
   { char filename[MMAP_LEN];
     strcpy(filename,MMAP_DIR);
     if(unique(filename,strchr(filename,'\0'),MMAP_LEN,MMAP_PERM,0,0)&&
	(mb->fd=ropen(filename,O_RDWR,MMAP_PERM),unlink(filename),mb->fd>=0))
      { if(lseek(mb->fd,mb->filelen=len,SEEK_SET)<0||1!=rwrite(mb->fd,empty,1))
dropf:	 { close(mb->fd);mb->fd= -1;
	   if(verbose)nlog("Unable to extend or use tempfile");
	 }
	else if(mb->len)
	 { long towrite,start,wrote;
	   if(lseek(mb->fd,0,SEEK_SET))
	      goto dropf;
	   for(start=0,towrite=mb->len>len?len:mb->len;towrite;)
	    { if(0>(wrote=rwrite(mb->fd,mb->p+start,towrite)))
		 goto dropf;
	      towrite-=wrote;start+=wrote;
	    }
	   free(mb->p);
	   mb->len=len;
	   goto mmap;
	 }
      }
   }
  if(mb->fd>=0)
   { if(len>mb->filelen)				  /* need to extend? */
      { if(lseek(mb->fd,mb->filelen=len,SEEK_SET)<0||1!=rwrite(mb->fd,empty,1))
	 { char*p=malloc(len+1);	   /* can't extend, switch to malloc */
	   memcpy(p,mb->p,mb->len);
	   munmap(mb->p,mb->len+1);
	   mb->len=len;
	   goto dropf;
	 }
	munmap(mb->p,mb->len+1);
mmap:	if((mb->p=mmap(0,len+1,P_RW,MAP_SHARED,mb->fd,0))==MAP_FAILED)
	 { static const char mmapfailed[]="Unable to mmap file";
	   nextexit=2;nlog(mmapfailed);elog("\n");
	   syslog(LOG_NOTICE,"%s of %ld bytes\n",mmapfailed,(long)len);
	   if(retval!=EX_TEMPFAIL)
	      retval=EX_OSERR;
	   Terminate();
	 }
      }
     mb->len=len;
     goto ret1;
   }
#endif
  if(nonfatal)
   { char*p;
     p=frealloc(mb->p,(size_t)(len+1));
     if(!p)
	return 0;
     mb->p=p;
   }
  else
     mb->p=realloc(mb->p,len+1);
  mb->len=len+1;
  mb->p[len]='\0';
ret1:
  return 1;
}

char*read2blk(mb,filledp,read_func,cleanup_func,data)memblk*const mb;
 read_func_type*read_func;cleanup_func_type*cleanup_func;
 long*const filledp;void*data;
{ int blksiz=BLKSIZ,ok;unsigned int shift=EXPBLKSIZ;
  long filled= *filledp,origfilled=filled;
  if(filled<mb->len)		 /* skip the initial resize if we have space */
     goto jumpin;
  for(;;)
   { if((size_t)filled>=(size_t)(filled+blksiz))       /* check for overflow */
	lcking|=lck_MEMORY,nomemerr(filled);
				       /* dynamically adjust the buffer size */
     while(EXPBLKSIZ&&(ok=0,blksiz>BLKSIZ)&&	   /* backed up all the way? */
      !(ok=resizeblock(mb,filled+blksiz,1)))	  /* no?  Then try this size */
	blksiz>>=1;			 /* failed!  Try a smaller increment */
     if(!EXPBLKSIZ||!ok)
	resizeblock(mb,filled+blksiz,0);	    /* last (maybe only) try */
jumpin:
     ;{ char*newlast;
	if(newlast=(*read_func)(mb->p+filled,mb->len-filled,data))
	 { filled=newlast-mb->p;
	   break;
	 }
	filled=mb->len;
      }
     if(EXPBLKSIZ&&shift)				 /* room for growth? */
      { int newbs=blksiz;newbs<<=shift--;	/* capped exponential growth */ if(blksiz<newbs)				  /* no overflowing? */
	   blksiz=newbs;				    /* yes, take me! */
      }
   }
  if(cleanup_func&&(*cleanup_func)(mb,&filled,origfilled,data))
     goto jumpin;
  resizeblock(mb,filled+1,1);		      /* minimise+1 for housekeeping */
  *filledp=filled;				 /* write back the new value */
  return mb->p;
}