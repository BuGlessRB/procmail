/************************************************************************
 *	setid		executes commands under a different		*
 *			uid/gid (can only be executed by root)		*
 *	This program is used by the SmartList installation script only. *
 ************************************************************************/
/*$Id: setid.c,v 1.7 1994/06/28 16:56:47 berg Exp $*/
#include "includes.h"

#define CHECK_FILE	"install.sh"

main(argc,argv)const int argc;const char*const argv[];
{ struct passwd*p;char*nargv[2];
  if(argc!=2&&argc!=3||geteuid()||!(p=getpwnam(argv[1])))
   { fprintf(stderr,"Usage: setid user [directory]\n");
     return EX_USAGE;
   }
  endpwent();initgroups(argv[1],p->pw_gid);setgid(p->pw_gid);setuid(p->pw_uid);
  if(fopen(CHECK_FILE,"r"))
   { struct stat stbuf;
     if(argc==2||!stat(argv[2],&stbuf)&&
	stbuf.st_uid==p->pw_uid&&
	stbuf.st_gid==p->pw_gid)
	nargv[0]=getenv("SHELL"),nargv[1]=0,execve(nargv[0],nargv,environ);
     else
	fprintf(stderr,
	 "Please create %s with the correct owner and group first\n",argv[2]);
   }
  else
     fprintf(stderr,
      "Please make sure %s can read & access the source tree\n",argv[1]);
  return EX_UNAVAILABLE;
}
