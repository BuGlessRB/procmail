/*$Id: sublib.c,v 1.2 1992/10/02 14:41:18 berg Exp $*/
#include "includes.h"
#include "sublib.h"

#ifdef NOmemmove
void*smemmove(To,From,count)void*To,*From;register size_t count;
#ifdef NObcopy
{ register char*to=To,*from=From;/*void*old;*/	  /* silly compromise, throw */
  /*old=to;*/count++;--to;--from;  /* away space to be syntactically correct */
  if(to<=from)
   { goto jiasc;
     do
      { *++to= *++from;					  /* copy from above */
jiasc:;
      }
     while(--count);
   }
  else
   { to+=count;from+=count;goto jidesc;
     do
      { *--to= *--from;					  /* copy from below */
jidesc:;
      }
     while(--count);
   }
  return To/*old*/;
#else
{ bcopy(From,To,count);return To;
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
  return(char*)f;
}
#endif
			    /* strtol replacement which lacks range checking */
#ifdef NOstrtol
long strtol(start,ptr,base)const char*start,**const ptr;
{ long result;const char*str=start;unsigned i;int sign,found;
  if(base>=36||base<(sign=found=result=0))
     goto fault;
  for(;;str++)					  /* skip leading whitespace */
   { switch(*str)
      { case '\t':case '\n':case '\v':case '\f':case '\r':case ' ':continue;
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
   { found=1;result=result*base+i;++str;		 /* start converting */
jumpin:
     if((i= *str-'0')<10);
     else if(i-'A'+'0'<='Z'-'A')
	i-='A'-10-'0';			   /* collating sequence dependency! */
     else if(i-'a'+'0'<'z'-'a')
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
#ifndef NOstrpbrk
#ifndef NOmemmove
int sublib_dummy_var;		      /* to prevent insanity in some linkers */
#endif
#endif
#endif /* NOstrtol */
