/************************************************************************
 *	Collection of standard library substitute routines		*
 *									*
 *	Copyright (c) 1990-1997, S.R. van den Berg, The Netherlands	*
 *	Copyright (c) 1999-2001, Philip Guenther, The United States	*
 *							of America	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: sublib.c,v 1.25 2001/06/23 08:18:51 guenther Exp $";
#endif
#include "includes.h"
#include "sublib.h"

#ifdef NOmemmove
void*smemmove(To,From,count)void*To;const void*From;register size_t count;
#ifdef NObcopy					  /* silly compromise, throw */
{ register char*to=To;register const char*from=From;/*void*old;*/
  /*old=to;*/count++;to--;from--;  /* away space to be syntactically correct */
  if(to<=from)
   { goto jiasc;
     do
      { *++to= *++from;					  /* copy from above */
jiasc:;
      }
     while(--count);
   }
  else
   { to+=count;from+=count;
     goto jidesc;
     do
      { *--to= *--from;					  /* copy from below */
jidesc:;
      }
     while(--count);
   }
  return To/*old*/;
#else
{ bcopy(From,To,count);
  return To;
#endif /* NObcopy */
}
#endif /* NOmemmove */

#include "shell.h"

#ifdef NOstrpbrk
char*strpbrk(st,del)const char*const st,*del;
{ const char*f=0,*t;
  for(f=0;*del;)
     if((t=strchr(st,*del++))&&(!f||t<f))
	f=t;
  return (char*)f;
}
#endif

#ifdef BENCHSIZE					     /* for autoconf */
#ifndef SLOWstrstr
#define SLOWstrstr
#else
#undef BENCHSIZE
#endif
#endif
#ifdef SLOWstrstr
/*
 *	My personal strstr() implementation that beats most other algorithms.
 *	Until someone tells me otherwise, I assume that this is the
 *	fastest implementation of strstr() in C.
 *	I deliberately chose not to comment it.	 You should have at least
 *	as much fun trying to understand it, as I had to write it :-).
 */
typedef unsigned chartype;

char*sstrstr(phaystack,pneedle)const char*const phaystack;
 const char*const pneedle;
{ register const uchar*haystack,*needle;register chartype b,c;
  haystack=(const uchar*)phaystack;
  if(b= *(needle=(const uchar*)pneedle))
   { haystack--;				  /* possible ANSI violation */
     do
	if(!(c= *++haystack))
	   goto ret0;
     while(c!=b);
     if(!(c= *++needle))
	goto foundneedle;
     ++needle;
     goto jin;
     for(;;)
      { ;{ register chartype a;
	   do
	    { if(!(a= *++haystack))
		 goto ret0;
	      if(a==b)
		 break;
	      if(!(a= *++haystack))
		 goto ret0;
shloop:;    }
	   while(a!=b);
jin:	   if(!(a= *++haystack))
	      goto ret0;
	   if(a!=c)
	      goto shloop;
	 }
	;{ register chartype a;
	   ;{ register const uchar*rhaystack,*rneedle;
	      if(*(rhaystack=haystack--+1)==(a= *(rneedle=needle)))
		 do
		  { if(!a)
		       goto foundneedle;
		    if(*++rhaystack!=(a= *++needle))
		       break;
		    if(!a)
		       goto foundneedle;
		  }
		 while(*++rhaystack==(a= *++needle));
	      needle=rneedle;		   /* took the register-poor aproach */
	    }
	   if(!a)
	      break;
	 }
      }
   }
foundneedle:
  return (char*)haystack;
ret0:
  return 0;
}
#endif

#ifdef NEEDbzero				      /* NObzero && NOmemset */
void bzero(s,n)void *s;size_t n;
{ register char*p=s;
  while(n-->0)
     *p++='\0';
}
#endif

#ifdef NOstrlcat
size_t strlcat(dst,src,size)char *dst;const char*src;size_t size;
{ const char*start=dst;
  if(size>0)
   { size--;					/* reserve space for the NUL */
     while(size>0&&*dst)				  /* skip to the end */
	size--,dst++;
     while(size>0&&*src)			     /* copy over characters */
	size--,*dst++= *src++;
     *dst='\0';					    /* hasta la vista, baby! */
   }
  return dst-start+strlen(src);
}
#endif

#ifdef NOstrerror
#define ERRSTR "Error number "
char *strerror(int err)
{ static char errbuf[STRLEN(ERRSTR)+sizeNUM(int)+1]=ERRSTR;
#ifndef NOsys_errlist
  if(err>=0&&err<sys_nerr)
     return sys_errlist[err];
#endif
  ultstr(0,(unsigned int)err,errbuf+STRLEN(ERRSTR));
  return errbuf;
}
#endif
			    /* strtol replacement which lacks range checking */
#ifdef NOstrtol
long strtol(start,ptr,base)const char*start,**const ptr;int base;
{ long result;const char*str=start;unsigned i;int sign,found;
  if(base<(sign=found=result=0)||base>=36)
     goto fault;
  for(;;str++)					  /* skip leading whitespace */
   { switch(*str)
      { case '\t':case '\n':case '\v':case '\f':case '\r':case ' ':
	   continue;
      }
     break;
   }
  switch(*str)						       /* any signs? */
   { case '-':sign=1;
     case '+':str++;
   }
  if(*str=='0')						 /* leading zero(s)? */
   { start++;
     if((i= *++str)=='x'||i=='X')			/* leading 0x or 0X? */
	if(!base||base==16)
	   base=16,str++;			    /* hexadecimal all right */
	else
	   goto fault;
     else if(!base)
	base=8;						 /* then it is octal */
   }
  else if(!base)
     base=10;						  /* or else decimal */
  goto jumpin;
  do
   { found=1;result=result*base+i;str++;		 /* start converting */
jumpin:
     if((i=(unsigned)*str-'0')<10);
     else if(i-'A'+'0'<='Z'-'A')
	i-='A'-10-'0';			   /* collating sequence dependency! */
     else if(i-'a'+'0'<='z'-'a')
	i-='a'-10-'0';			   /* collating sequence dependency! */
     else
	break;						/* not of this world */
   }
  while(i<base);				      /* still of this world */
fault:
  if(ptr)
    *ptr=found?str:start;			       /* how far did we get */
  return sign?-result:result;
}
#else /* NOstrtol */
#ifndef NOstrerror
#ifndef NOstrlcat
#ifndef NEEDbzero
#ifndef SLOWstrstr
#ifndef NOstrpbrk
#ifndef NOmemmove
int sublib_dummy_var;		      /* to prevent insanity in some linkers */
#endif
#endif
#endif
#endif
#endif
#endif
#endif /* NOstrtol */
