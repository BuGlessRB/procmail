/************************************************************************
 *	recommend	Analyses the installation, and makes		*
 *			recommendations about suid/sgid modes		*
 ************************************************************************/
/*$Id: recommend.c,v 1.7 1994/05/26 13:48:17 berg Exp $*/
#include "includes.h"				       /* also for fprintf() */

#ifndef SYSTEM_MBOX
#define SYSTEM_MBOX	SYSTEM_MAILBOX
#endif

#define PERMIS	(S_IRWXU|S_IRWXG&~S_IWGRP|S_IRWXO&~S_IWOTH)

char systm_mbox[]=SYSTEM_MBOX;
const char dirsep[]=DIRSEP,
 *const checkf[]={"/bin/mail","/bin/lmail","/usr/lib/sendmail",
 "/usr/lib/smail",0};
				     /* following routine lifted from misc.c */
char*lastdirsep(filename)const char*filename;	 /* finds the next character */
{ const char*p;					/* following the last DIRSEP */
  while(p=strpbrk(filename,dirsep))
     filename=p+1;
  return(char*)filename;
}

main(argc,argv)const int argc;const char*const argv[];
{ struct group*grp;struct stat stbuf;gid_t gid=(gid_t)-1;
  const char*const*p;mode_t sgid=0;int chmdir=0;
  if(argc!=3)
   { fprintf(stderr,"Please run this program via 'make recommend'\n");
     return EX_USAGE;
   }
  *lastdirsep(systm_mbox)='\0';
  for(p=checkf;*p;p++)
     if(!stat(*p,&stbuf)&&stbuf.st_mode&S_ISGID)
      { if(stbuf.st_mode&S_ISGID)
	   sgid=S_ISGID,gid=stbuf.st_gid;
	break;
      }
  if(!stat(systm_mbox,&stbuf)&&!(stbuf.st_mode&S_IWOTH))
   { sgid=S_ISGID;gid=stbuf.st_gid;
     if(!(stbuf.st_mode&S_IWGRP))
	chmdir=1;
   }
  if(gid!=stbuf.st_gid)
     sgid=0;
  printf("chown root %s\n",argv[1]);
  if(sgid)
     if(grp=getgrgid(gid))
	printf("chgrp %s %s %s\n",grp->gr_name,argv[1],argv[2]);
     else
	printf("chgrp %u %s %s\n",(int)gid,argv[1],argv[2]);
  printf("chmod %o %s\n",sgid|S_ISUID|PERMIS,argv[1]);
  if(sgid)
   { printf("chmod %o %s\n",sgid|PERMIS,argv[2]);
     if(chmdir)
	printf("chmod g+w %s.\n",systm_mbox);
   }
  return EX_OK;
}
