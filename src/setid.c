/************************************************************************
 *	setid		executes commands under a different		*
 *			uid/gid (can only be executed by root)		*
 ************************************************************************/
/*$Id: setid.c,v 1.1 1994/03/10 17:15:28 berg Exp $*/
#include "includes.h"				       /* also for fprintf() */

main(argc,argv)const int argc;const char*const argv[];
{ struct passwd*p;char*nargv[2];
  if(argc!=2||geteuid()||!(p=getpwnam(argv[1])))
   { fprintf("Usage: setid user\n");return EX_USAGE;
   }
  endpwent();initgroups(argv[1],p->pw_gid);setgid(p->pw_gid);setuid(p->pw_uid);
  nargv[0]=getenv("SHELL");nargv[1]=0;execve(nargv[0],nargv,environ);
  return EX_UNAVAILABLE;
}
