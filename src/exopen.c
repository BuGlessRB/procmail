/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static char rcsid[]="$Id: exopen.c,v 1.4 1992/09/30 17:55:37 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "misc.h"
#include "exopen.h"

const char*hostname()
{ static char name[HOSTNAMElen+1];
#ifdef	NOuname
  gethostname(name,HOSTNAMElen+1);
#else
  struct utsname names;
  Uname(&names);strncpy(name,names.nodename,HOSTNAMElen);
#endif
  name[HOSTNAMElen]='\0';return name;
}

void ultoan(val,dest)unsigned long val;char*dest;     /* convert to a number */
{ register i;				     /* within the set [0-9A-Za-z-_] */
  do
   { i=val&0x3f;			   /* collating sequence dependency! */
     *dest++=i+(i<10?'0':i<10+26?'A'-10:i<10+26+26?'a'-10-26:
      i==10+26+26?'-'-10-26-26:'_'-10-26-27);
   }
  while(val>>=6);
  *dest='\0';
}

unique(full,p,mode)const char*const full;char*const p;const mode_t mode;
{ unsigned long retry=mrotbSERIAL;int i;	  /* create unique file name */
  do
   { ultoan(maskSERIAL&(retry<<bitsSERIAL-mrotbSERIAL)+
      (unsigned long)thepid,p+1);
     *p=UNIQ_PREFIX;strcat(p,hostname());
   }
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
  while(0>(i=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST&&
   retry--);	    /* casually check if it already exists (highly unlikely) */
  if(i<0)
   { writeerr(full);return 0;
   }
  rclose(i);return 1;
}
				     /* rename MUST fail if already existent */
myrename(old,newn)const char*const old,*const newn;
{ int i,serrno;struct stat stbuf;
  link(old,newn);serrno=errno;i=stat(old,&stbuf);unlink(old);errno=serrno;
  return stbuf.st_nlink==2?i:-1;
}
