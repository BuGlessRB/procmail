/************************************************************************
 *	setid		executes commands under a different		*
 *			uid/gid (can only be executed by root)		*
 *	This program is used by the SmartList installation script only. *
 ************************************************************************/
/*$Id: setid.c,v 1.8 1994/10/14 18:43:49 berg Exp $*/
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
     if(argc==2)
	goto nodir;
     if(stat(argv[2],&stbuf)||(stbuf.st_mode&S_IRWXU)!=S_IRWXU)
	fprintf(stderr,"Can't access %s, please create it first\n",argv[2]);
     else if(stbuf.st_uid!=p->pw_uid)
	fprintf(stderr,"%s is owned by uid %ld!=%s, please fix this first\n",
	 (long)stbuf.st_uid,p->pw_name);
     else if(stbuf.st_gid!=p->pw_gid)
	fprintf(stderr,"%s is owned by gid %ld!=%ld, please fix this first\n",
	 (long)stbuf.st_gid,(long)p->pw_gid);
     else
nodir:	nargv[0]=getenv("SHELL"),nargv[1]=0,execve(nargv[0],nargv,environ);
   }
  else
     fprintf(stderr,
      "Please make sure %s can read & access the source tree\n",argv[1]);
  return EX_UNAVAILABLE;
}
