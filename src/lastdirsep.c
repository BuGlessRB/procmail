/************************************************************************
 *	A file just for lastdirsep()					*
 *									*
 *	Copyright (c) 1994-1996, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: lastdirsep.c,v 1.3 1996/12/21 03:28:26 srb Exp $";
#endif
#include "includes.h"
#include "lastdirsep.h"

extern const char dirsep[];

char*lastdirsep(filename)const char*filename;	 /* finds the next character */
{ const char*p;					/* following the last DIRSEP */
  while(p=strpbrk(filename,dirsep))
     filename=p+1;
  return (char*)filename;
}
