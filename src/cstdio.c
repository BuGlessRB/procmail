/************************************************************************
 *	Custom standard-io library					*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static char rcsid[]="$Id: cstdio.c,v 1.1 1992/09/28 14:28:03 berg Exp $";
#endif
#include "procmail.h"
#include "cstdio.h"

static uchar rcbuf[STDBUF],*rcbufp,*rcbufend;	 /* buffers for custom stdio */
static long blasttell;
static struct dyna_long inced;				  /* includerc stack */

void pushrc(name)const char*const name;		      /* open include rcfile */
{ app_val(&inced,rcbufp?(long)(rcbufp-rcbuf):0L);app_val(&inced,blasttell);
  app_val(&inced,(long)rc);			 /* save old position and fd */
  if(bopen(name)<0)			      /* and try to open the new one */
   { nlog(couldnread);logqnl(name);poprc();	/* we couldn't, so restore rc */
   }
}

poprc()
{ rclose(rc);					     /* close it in any case */
  if(!inced.filled)				  /* include stack is empty? */
     return 0;	      /* restore rc, seekpos, prime rcbuf and restore rcbufp */
  rc=inced.offs[--inced.filled];lseek(rc,inced.offs[--inced.filled],SEEK_SET);
  rcbufp=rcbufend;getb();rcbufp=rcbuf+inced.offs[--inced.filled];return 1;
}

closerc()						/* {while(poprc());} */
{ while(rclose(rc),inced.filled)
     rc=inced.offs[inced.filled-1],inced.filled-=3;
}

bopen(name)const char*const name;				 /* my fopen */
{ rcbufp=rcbufend=0;return rc=ropen(name,O_RDONLY,0);
}

getbl(p)char*p;							  /* my gets */
{ int i;char*q;
  for(q=p;;)
   { switch(i=getb())
      { case '\n':case EOF:
	   *p='\0';return p!=q;		     /* did we read anything at all? */
      }
     *p++=i;
   }
}

getb()								 /* my fgetc */
{ if(rcbufp==rcbufend)						   /* refill */
     blasttell=tell(rc),rcbufend=rcbuf+rread(rc,rcbufp=rcbuf,STDBUF);
  return rcbufp<rcbufend?*rcbufp++:EOF;
}

void ungetb(x)const int x;	/* only for pushing back original characters */
{ if(x!=EOF)
     rcbufp--;							   /* backup */
}

testb(x)const int x;		   /* fgetc that only succeeds if it matches */
{ int i;
  if((i=getb())==x)
     return 1;
  ungetb(i);return 0;
}

sgetc()						/* a fake fgetc for a string */
{ return *sgetcp?*(uchar*)sgetcp++:EOF;
}

skipspace()
{ int any=0;
  while(testb(' ')||testb('\t'))
     any=1;
  return any;
}
