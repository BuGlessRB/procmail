/************************************************************************
 *	Custom standard-io library					*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: cstdio.c,v 1.26 1994/06/28 16:56:03 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "cstdio.h"
#include "misc.h"

static uchar rcbuf[STDBUF],*rcbufp,*rcbufend;	  /* buffer for custom stdio */
static off_t blasttell;
static struct dyna_long inced;				  /* includerc stack */
struct dynstring*incnamed;

void pushrc(name)const char*const name;		      /* open include rcfile */
{ struct stat stbuf;					   /* only if size>0 */
  if(*name&&(stat(name,&stbuf)||!S_ISREG(stbuf.st_mode)||stbuf.st_size))
   { app_val(&inced,rcbufp?(off_t)(rcbufp-rcbuf):(off_t)0);	 /* save old */
     app_val(&inced,blasttell);app_val(&inced,(off_t)rc);   /* position & fd */
     if(bopen(name)<0)			      /* and try to open the new one */
	readerr(name),poprc();		       /* we couldn't, so restore rc */
   }
}

void duprcs P((void))
{ size_t i;struct dynstring*dp;
  dp=incnamed;rclose(rc);
  if(0>(rc=ropen(dp->ename,O_RDONLY,0)))
     goto dupfailed;
  lseek(rc,blasttell+STDBUF,SEEK_SET);
  for(i=inced.filled;dp=dp->enext,i;i-=2)
   { int fd;
     rclose(inced.offs[--i]);
     if(0>(fd=ropen(dp->ename,O_RDONLY,0)))
dupfailed:
	nlog("Lost"),logqnl(dp->ename),exit(EX_NOINPUT);
     inced.offs[i]=fd;
   }
}

static void closeonerc P((void))
{ struct dynstring*last;
  if(rc>=0)
     rclose(rc),rc= -1,last=incnamed,incnamed=last->enext,free(last);
}

int poprc P((void))
{ closeonerc();					     /* close it in any case */
  if(skiprc)
     skiprc=0,nlog("Missing closing brace\n");
  if(!inced.filled)				  /* include stack is empty? */
     return 0;	      /* restore rc, seekpos, prime rcbuf and restore rcbufp */
  rc=inced.offs[--inced.filled];lseek(rc,inced.offs[--inced.filled],SEEK_SET);
  rcbufp=rcbufend;getb();rcbufp=rcbuf+inced.offs[--inced.filled];
  return 1;
}

void closerc P((void))					/* {while(poprc());} */
{ while(closeonerc(),inced.filled)
     rc=inced.offs[inced.filled-1],inced.filled-=3;
}
							    /* destroys buf2 */
int bopen(name)const char*const name;				 /* my fopen */
{ rcbufp=rcbufend=0;rc=ropen(name,O_RDONLY,0);
  if(rc>=0)
   { char*md;		 /* if it's a relative name and an absolute $MAILDIR */
     if(!strchr(dirsep,*name)&&strchr(dirsep,*(md=(char*)tgetenv(maildir))))
      { strcpy(buf2,md);*(md=strchr(buf2,'\0'))= *dirsep;strcpy(++md,name);
	md=buf2;				    /* then prepend $MAILDIR */
      }
     else
	md=(char*)name;			      /* pick the original otherwise */
     newdynstring(&incnamed,md);
   }
  return rc;
}

int getbl(p)char*p;						  /* my gets */
{ int i;char*q;
  for(q=p;;)
   { switch(i=getb())
      { case '\n':case EOF:*q='\0';
	   return p!=q;			     /* did we read anything at all? */
      }
     *q++=i;
   }
}

int getb P((void))						 /* my fgetc */
{ if(rcbufp==rcbufend)						   /* refill */
   { blasttell=tell(rc);rcbufend=rcbuf+rread(rc,rcbufp=rcbuf,STDBUF);
     if(rcbufp==rcbufend)
	rcbufp++;
   }
  return rcbufp<rcbufend?(int)*rcbufp++:EOF;
}

void ungetb(x)const int x;	/* only for pushing back original characters */
{ if(x!=EOF)
     rcbufp--;							   /* backup */
}

int testb(x)const int x;	   /* fgetc that only succeeds if it matches */
{ int i;
  if((i=getb())==x)
     return 1;
  ungetb(i);
  return 0;
}

int sgetc P((void))				/* a fake fgetc for a string */
{ return *sgetcp?(int)*(uchar*)sgetcp++:EOF;
}

int skipspace P((void))
{ int any=0;
  while(testb(' ')||testb('\t'))
     any=1;
  return any;
}

void getlline(target)char*target;
{ char*chp2;
  for(;getbl(chp2=target)&&*(target=strchr(target,'\0')-1)=='\\';
   *target++='\n')					   /* read line-wise */
     if(chp2!=target)					  /* non-empty line? */
	target++;			      /* then preserve the backslash */
}
