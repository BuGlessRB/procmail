/************************************************************************
 *	Collection of NFS resistant exclusive creat routines		*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: exopen.c,v 1.14 1993/11/24 19:46:18 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "misc.h"
#include "exopen.h"

const char*hostname P((void))
{ static char name[HOSTNAMElen+1];
#ifdef	NOuname
  gethostname(name,HOSTNAMElen+1);
#else
  struct utsname names;
  Uname(&names);strncpy(name,names.nodename,HOSTNAMElen);
#endif
  name[HOSTNAMElen]='\0';return name;
}

static volatile gotsig;

static void fakehandler P((void))
{ gotsig=1;
}

void qsignal(sig,action)const int sig;void(*action)P((void));
{ gotsig=0;
  if(SIG_IGN==signal(sig,(void(*)())fakehandler))
     signal(sig,SIG_IGN);
  else
   { signal(sig,(void(*)())action);
     if(gotsig)
	(*action)();
   }
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

unique(full,p,mode,verbos)const char*const full;char*const p;
 const mode_t mode;const int verbos;
{ unsigned long retry=mrotbSERIAL;int i;	  /* create unique file name */
  do
   { ultoan(maskSERIAL&(retry-=irotbSERIAL)+(long)thepid,p+1);*p=UNIQ_PREFIX;
     strcat(p,hostname());
   }
#ifndef O_CREAT
#define ropen(path,type,mode)	creat(path,mode)
#endif
  while(0>(i=ropen(full,O_WRONLY|O_CREAT|O_EXCL,mode))&&errno==EEXIST&&
   retry);	    /* casually check if it already exists (highly unlikely) */
  if(i<0)
   { if(verbos)			      /* this error message can be confusing */
	writeerr(full);					 /* for casual users */
     return 0;
   }
  rclose(i);return 1;
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
     serrno=errno;i=stat(old,&stbuf);errno=serrno;
     if(i)
	goto retfail;
     if(stbuf.st_nlink!=2)
      { if(serrno!=EXDEV)		       /* failure due to filesystem? */
	   goto retfail;		     /* try it by conventional means */
	if(0>(i=ropen(newn,O_WRONLY|O_CREAT|O_EXCL,stbuf.st_mode)))
	 { rclose(i);
retfail:   return -1;
	 }
      }
   }
  return 0;
}
