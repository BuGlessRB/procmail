/************************************************************************
 *	Some common routines for procmail and formail			*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static char rcsid[]="$Id: common.c,v 1.1 1992/09/28 14:28:03 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "shell.h"
#include "common.h"

void shexec(argv)char*const*argv;
{ int i;char**newargv;const char**p;
#ifdef SIGXCPU
  signal(SIGXCPU,SIG_DFL);signal(SIGXFSZ,SIG_DFL);
#endif
#ifdef SIGLOST
  signal(SIGLOST,SIG_DFL);
#endif
  signal(SIGPIPE,SIG_DFL);execvp(*argv,argv);	/* or is it a shell script ? */
  for(p=(const char**)argv,i=1;i++,*p++;);	      /* count the arguments */
  newargv=malloc(i*sizeof*p);
  for(*(p=(const char**)newargv)=binsh;*++p= *argv++;);
  execve(*newargv,newargv,environ);	      /* no shell script? -> trouble */
  nlog("Failed to execute");logqnl(*argv);exit(EX_UNAVAILABLE);
}

void detab(p)char*p;
{ while(p=strchr(p,'\t'))
     *p=' ';						/* take out all tabs */
}

char*pstrspn(whole,sub)const char*whole,*const sub;
{ while(*whole&&strchr(sub,*whole))
     whole++;
  return(char*)whole;
}

#ifdef NOstrcspn
strcspn(whole,sub)const char*const whole,*const sub;
{ const register char*p;
  p=whole;
  while(*p&&!strchr(sub,*p))
     p++;
  return p-whole;
}
#endif

void ultstr(minwidth,val,dest)unsigned long val;char*dest;
{ int i;unsigned long j;
  j=val;i=0;					   /* a beauty, isn't it :-) */
  do i++;					   /* determine needed width */
  while(j/=10);
  while(--minwidth>=i)				 /* fill up any excess width */
     *dest++=' ';
  *(dest+=i)='\0';
  do *--dest='0'+val%10;			  /* display value backwards */
  while(val/=10);
}

strnicmp(a,b,l)register const char*a,*b;register unsigned l;
{ unsigned i,j;
  if(l)						 /* case insensitive strncmp */
     do
      { while(*a&&*a==*b&&--l)
	   ++a,++b;
	if(!l)
	   break;
	if((i= *a++)-'A'<='Z'-'A')
	   i+='a'-'A';
	if((j= *b++)-'A'<='Z'-'A')
	   j+='a'-'A';
	if(j!=i)
	   return i>j?1:-1;
      }
     while(i&&j&&--l);
  return 0;
}
