/************************************************************************
 *	Some common routines to all programs but formail		*
 *									*
 *	Copyright (c) 1993-1996, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: mcommon.c,v 1.4 1996/12/21 03:28:29 srb Exp $";
#endif
#include "includes.h"
#include "mcommon.h"

static volatile int gotsig;

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
