/************************************************************************
 *	subhome		Output the subtree starting at the home dir	*
 *			of the list user.				*
 *	This program is used by the SmartList installation script only. *
 ************************************************************************/
/*$Id: gethome.c,v 1.8 1999/04/19 06:42:16 guenther Exp $*/
#include "includes.h"

int main(argc,argv)const int argc;const char*const argv[];
{ struct stat homest;
  ;{ struct passwd*p;
     if(argc!=3||!(p=getpwnam(argv[1])))
      { fprintf(stderr,"Usage: gethome user directory\n");
	return EX_USAGE;
      }
     endpwent();
     if(stat(p->pw_dir,&homest))
      { fprintf(stderr,"Couldn't find home directory %s\n",p->pw_dir);
	return EX_NOINPUT;
      }
   }
  ;{ const char*first;char*chp;
     for(chp=strchr(first=argv[2],'\0');;)
      { ;{ int res;struct stat walk;
	   ;{ int save;			     /* temporarily chop off the end */
	      save= *chp;*chp='\0';res=stat(first,&walk);*chp=save;
	    }
#define EQ(what)       (homest.what==walk.what)
	   if(!res&&EQ(st_dev)&&EQ(st_ino)&&EQ(st_uid)&&EQ(st_gid))
	    { if(*chp)
		 chp++;
	      break;				      /* found the directory */
	    }
	 }
	do
	   if(chp==first)			   /* sorry, end of the line */
	      goto breakout;
	while(!strchr(DIRSEP,*--chp));	/* search backward for the separator */
      }
breakout:
     printf("%s\n",chp);    /* this should be relative to the home directory */
   }
  return EXIT_SUCCESS;
}
