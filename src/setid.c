/************************************************************************
 *	setid		executes commands under a different		*
 *			uid/gid (can only be executed by root)		*
 *	This program is used by the SmartList installation script only. *
 ************************************************************************/
/*$Id: setid.c,v 1.3 1994/04/08 15:22:43 berg Exp $*/
#include "includes.h"				       /* also for fprintf() */

main(argc,argv)const int argc;const char*const argv[];
{ struct passwd*p;char*nargv[2];
  if(argc!=2&&argc!=3||geteuid()||!(p=getpwnam(argv[1])))
   { fprintf(stderr,"Usage: setid user [directory]\n");return EX_USAGE;
   }
  endpwent();initgroups(argv[1],p->pw_gid);setgid(p->pw_gid);setuid(p->pw_uid);
  ;{ struct stat stbuf;
     if(argc==2||!stat(argv[2],&stbuf)&&
	stbuf.st_uid==p->pw_uid&&
	stbuf.st_gid==p->pw_gid)
	nargv[0]=getenv("SHELL"),nargv[1]=0,execve(nargv[0],nargv,environ);
     else
	fprintf(stderr,
	 "Please create %s with the correct owner and group first\n",argv[2]);
   }
  return EX_UNAVAILABLE;
}
