#include "includes.h"
#include "ecommon.h"
#include "common.h"

static const char outofmem[]="Out of memory\n";

void*tmalloc(len)const size_t len;
{ void*p;
  if(p=malloc(len))
     return p;
  nlog(outofmem);exit(EX_OSERR);
}

void*trealloc(old,len)void*old;const size_t len;
{ if(old=realloc(old,len))
     return old;
  nlog(outofmem);exit(EX_OSERR);
}

void tfree(a)void*a;
{ free(a);
}

#include "shell.h"

mystrstr(whole,part,end)const char*whole,*const part,*end;
{ size_t i;
  for(end-=(i=strlen(part))+1;--end>=whole;)
     if(!strnIcmp(end,part,i))
	return 1;
  return 0;
}
