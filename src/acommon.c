/************************************************************************
 *	Some routine common to procmail, formail and lockfile		*
 *									*
 *	Copyright (c) 1993-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: acommon.c,v 1.2 1994/06/28 16:55:56 berg Exp $";
#endif
#include "includes.h"
#include "acommon.h"
#include "robust.h"
#include "shell.h"

const char*hostname P((void))
{
#ifdef	NOuname
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif
  static char name[MAXHOSTNAMELEN];
  gethostname(name,MAXHOSTNAMELEN);name[MAXHOSTNAMELEN-1]='\0';
#else
  struct utsname names;static char*name;
  if(name)
     free(name);
  Uname(&names);
  if(!(name=malloc(strlen(names.nodename)+1)))
     return "";		      /* can happen when called from within lockfile */
  strcpy(name,names.nodename);
#endif
  return name;
}

char*ultoan(val,dest)unsigned long val;char*dest;     /* convert to a number */
{ register i;				     /* within the set [0-9A-Za-z-_] */
  do
   { i=val&0x3f;			   /* collating sequence dependency! */
     *dest++=i+(i<10?'0':i<10+26?'A'-10:i<10+26+26?'a'-10-26:
      i==10+26+26?'-'-10-26-26:'_'-10-26-27);
   }
  while(val>>=6);
  *dest='\0';
  return dest;
}
