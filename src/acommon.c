/************************************************************************
 *	Some routine common to procmail, formail and lockfile		*
 *									*
 *	Copyright (c) 1993-1997, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: acommon.c,v 1.9 1999/06/09 07:44:19 guenther Exp $";
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
  static char name[MAXHOSTNAMELEN]="";
  if(!name[0])
     gethostname(name,MAXHOSTNAMELEN),name[MAXHOSTNAMELEN-1]='\0';
#else
  static char*name=0;
  if(!name)
   { struct utsname names;
     Uname(&names);
     if(!(name=malloc(strlen(names.nodename)+1)))
	return "";	      /* can happen when called from within lockfile */
     strcpy(name,names.nodename);
   }
#endif
  return name;
}

char*ultoan(val,dest)unsigned long val;char*dest;     /* convert to a number */
{ register int i;			     /* within the set [A-Za-z0-9-_] */
  do
   { i=val&0x3f;			   /* collating sequence dependency! */
     *dest++=i+(i<26?'A':i<26+26?'a'-26:i<26+26+10?'0'-26-26:
      i==26+26+10?'-'-26-26-10:'_'-26-26-11);
   }
  while(val>>=6);
  *dest='\0';
  return dest;
}
