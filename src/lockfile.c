/************************************************************************
 *	lockfile - The conditional semaphore-file creator		*
 *									*
 *	It has been designed to be able to be run sgid mail or		*
 *	any gid you see fit (in case your mail spool area is *not*	*
 *	world writeable, but group writeable), without creating		*
 *	security holes.							*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: lockfile.c,v 1.15 1993/06/21 14:24:28 berg Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 1993/06/21 14:24:28 $";
#include "includes.h"
#include "sublib.h"
#include "exopen.h"

#ifndef SYSTEM_MBOX
#define SYSTEM_MBOX	SYSTEM_MAILBOX
#endif

static volatile exitflag;
pid_t thepid;
static char systm_mbox[]=SYSTEM_MBOX;
static const char dirsep[]=DIRSEP,lockext[]=DEFlockext,
 nameprefix[]="lockfile: ",lgname[]="LOGNAME",home[]="HOME";

static void failure P((void))				      /* signal trap */
{ exitflag=2;					       /* merely sets a flag */
}
				    /* see locking.c for comment on xcreat() */
static xcreat(name,tim)const char*const name;time_t*const tim;
{ char*p,*q;int j= -1,i;struct stat stbuf;
  for(q=(char*)name;p=strpbrk(q,dirsep);q=p+1);
  i=q-name;
  if(!(p=malloc(i+UNIQnamelen)))
     return exitflag=1;
  strncpy(p,name,i);
  if(unique(p,p+i,0,0))
     stat(p,&stbuf),*tim=stbuf.st_mtime,j=myrename(p,name);
  free(p);return j;
}

void elog(a)const char*const a;
{ write(STDERR,a,strlen(a));
}

void nlog(a)const char*const a;
{ elog(nameprefix);elog(a);  /* decent error messages should start with this */
}

static size_t parsecopy(dest,org,pass)char*const dest;const char*org;
 const struct passwd*const pass; /* try and digest the mailbox-lockfile name */
{ size_t len;const char*chp;char*p;unsigned i;
  for(p=dest,len=STRLEN(lockext)+1;;)
   { switch(*org)
      { case '$':					    /* we substitute */
	   if(!strncmp(++org,lgname,STRLEN(lgname)))
	      org+=STRLEN(lgname),chp=pass->pw_name;	     /* $LOGNAME and */
	   else if(!strncmp(org,home,STRLEN(home)))
	      org+=STRLEN(home),chp=pass->pw_dir;		    /* $HOME */
	   else
	      goto capac;			     /* no other fancy stuff */
	   if((i= *org)-'a'<='z'-'a'||i-'A'<='Z'-'A'||numeric(i)||i=='_')
	      goto capac;
	   if(p)
	      p+=strlen(strcpy(p,chp));			      /* paste it in */
	   len+=strlen(chp);continue;
	default:
	   if(p)
	      *p++= *org;		      /* simply copy everything else */
	   len++;org++;continue;      /* except suspicous looking characters */
	case '\'':case '`':case '"':case '\\':case '{':goto capac;
	case '\0':;
      }
     if(p)
      { if(p==dest||!strchr(dirsep,*dest))	     /* absolute path wanted */
capac:	 { nlog("Sorry, but turning this mess into a useable mailbox \
exceeds my humble\ncapacities");return 0;
	 }
	strcpy(p,lockext);
      }
     return len;
   }
}

static PROGID;

main(argc,argv)const char*const argv[];
{ const char*const*p,*const*lastf;char*cp;uid_t uid;
  int sleepsec,retries,invert,force,suspend,retval=EX_OK,virgin=1;
  static const char usage[]="Usage: lockfile -nnn | -r nnn | -l nnn | -s nnn \
| -! | -ml | -mu | file ...\n";
  if(argc<=1)			       /* sanity check, any argument at all? */
     goto usg;
  sleepsec=DEFlocksleep;force=invert=(char*)progid-(char*)progid;retries= -1;
  suspend=DEFsuspend;thepid=getpid();uid=getuid();signal(SIGPIPE,SIG_IGN);
again:
  signal(SIGHUP,(void(*)())failure);signal(SIGINT,(void(*)())failure);
  signal(SIGQUIT,(void(*)())failure);signal(SIGTERM,(void(*)())failure);
  for(lastf=p=argv;--argc;)
     if(*(cp=(char*)*++p)=='-')
	for(cp++;;)
	 { char*cp2=cp;int i;
	   switch(*cp++)
	    { case '!':invert^=1;continue;	      /* invert the exitcode */
	      case 'r':case 'l':case 's':
		 if(!*cp&&(cp=(char*)*++p,!--argc)) /* concatenated/seperate */
		    goto eusg;
		 i=strtol(cp,&cp,10);
		 switch(*cp2)
		  { case 'r':retries=i;goto checkrdec;
		    case 'l':force=i;goto checkrdec;
		    default:
		       if(i<0)			    /* suspend should be >=0 */
			  goto eusg;
		       suspend=i;goto checkrdec;
		  }
	      case HELPOPT1:case HELPOPT2:elog(usage);
		 elog(
 "\t-nnn\twait nnn seconds between locking attempts\
\n\t-r nnn\tmake at most nnn retries before giving up on a lock\
\n\t-l nnn\tset locktimeout to nnn seconds\
\n\t-s nnn\tsuspend nnn seconds after a locktimeout occurred\
\n\t-!\tinvert the exit code of lockfile\
\n\t-ml\tlock your system mail-spool file\
\n\t-mu\tunlock your system mail-spool file\n");goto xusg;
	      default:
		 if(sleepsec>=0)	    /* is this still the first pass? */
		  { if((sleepsec=strtol(cp2,&cp,10))<0)
		       goto eusg;
checkrdec:	    if(cp2==cp)
eusg:		     { elog(usage);		    /* regular usage message */
xusg:		       retval=EX_USAGE;goto nfailure;
		     }
		  }
		 else		      /* no second pass, so leave sleepsec<0 */
		    strtol(cp2,&cp,10);		   /* and discard the number */
		 continue;
	      case 'm':		  /* take $LOGNAME as a hint, check if valid */
	       { struct passwd*pass;static char*ma;size_t alen;
		 if(*cp&&cp[1]||ma&&sleepsec>=0)	     /* second pass? */
		    goto eusg;
		 if(!ma)			/* ma initialised last time? */
		  { if(!((ma=(char*)getenv(lgname))&&(pass=getpwnam(ma))&&
		     pass->pw_uid==uid||(pass=getpwuid(uid))))
		     { nlog("Can't determine your mailbox, who are you?\n");
		       goto nfailure;	 /* panic, you're not in /etc/passwd */
		     }
		    if(!(alen=parsecopy((char*)0,systm_mbox,pass)))
		     { cp=systm_mbox;goto lfailure;	  /* couldn't digest */
		     }						  /* mailbox */
		    if(!(ma=malloc(alen)))	       /* ok, make some room */
		       goto outofmem;
		    parsecopy(ma,systm_mbox,pass);	  /* and fill her up */
		  }
		 switch(*cp)
		  { default:goto eusg;		    /* somebody goofed again */
		    case 'l':				 /* lock the mailbox */
		       if(sleepsec>=0)			      /* first pass? */
			{ cp=ma;goto stilv;		    /* yes, lock it! */
			}
		    case 'u':			       /* unlock the mailbox */
		       if(unlink(ma))
			{ nlog("Can't unlock \"");elog(ma);elog("\"");
			  if(*cp=='l')	 /* they messed up, give them a hint */
			     elog(" again,\n already dropped my privileges");
			  elog("\n");
			}
		       else
			  virgin=0;
		  }
	       }
	      case '\0':;
	    }
	   break;
	 }
     else if(sleepsec<0)      /* second pass, release everything we acquired */
	unlink(cp);
     else
      { time_t t;int permanent;
	setgid(getgid());		      /* just to be on the safe side */
stilv:	virgin=0;permanent=nfsTRY;
	while(0>xcreat(cp,&t))				     /* try and lock */
	 { struct stat stbuf;
	   if(exitflag)					    /* time to stop? */
	    { if(exitflag==1)		     /* was it failure() or malloc() */
outofmem:	 retval=EX_OSERR,nlog("Out of memory");
	      else
		 retval=EX_TEMPFAIL,nlog("Signal received");
	      goto lfailure;
	    }
	   switch(errno)		    /* why did the lock not succeed? */
	    { case EEXIST:			  /* hmmm..., by force then? */
		 if(force&&!lstat(cp,&stbuf)&&force<t-stbuf.st_mtime)
		  { nlog(unlink(cp)?"Forced unlock denied on \"":
		     "Forcing lock on \"");
		    elog(cp);elog("\"\n");sleep(suspend);	/* important */
		  }
		 else					   /* no forcing now */
	      case ENOSPC:
#ifdef EDQUOT
	      case EDQUOT:
#endif
		    switch(retries)    /* await your turn like everyone else */
		     { case 0:nlog("Sorry");retval=EX_CANTCREAT;
			  goto lfailure;      /* patience exhausted, give up */
		       default:retries--;		      /* count sheep */
		       case -1:sleep(sleepsec);		     /* wait and see */
		     }
		 break;
	      case ENOENT:case ENOTDIR:case EIO:case EACCES:
		 if(!--permanent)	 /* NFS sporadically generates these */
		  { sleep(sleepsec);continue;		      /* unwarranted */
		  }				     /* so ignore them first */
	      default:		     /* but, it seems to persist, so give up */
		 nlog("Try praying");retval=EX_UNAVAILABLE;
#ifdef ENAMETOOLONG
		 goto lfailure;
	      case ENAMETOOLONG:
		 if(0<(permanent=strlen(cp)-1)&&      /* can we truncate it? */
		  !strchr(dirsep,cp[permanent-1]))
		  { nlog("Truncating \"");elog(cp);	      /* then try it */
		    elog("\" and retrying lock\n");cp[permanent]='\0';break;
		  }				     /* otherwise, forget it */
		 nlog("Filename too long");retval=EX_UNAVAILABLE;
#endif
lfailure:	 elog(", giving up on \"");elog(cp);elog("\"\n");
nfailure:	 sleepsec= -1;argc=lastf-argv+1;goto again; /* mark sleepsec */
	    }  /* for second pass, and adjust argc to the no. of args parsed */
	   permanent=nfsTRY;	       /* refresh the NFS-error-ignore count */
	 }
	lastf=p;					  /* last valid file */
      }
  if(retval==EX_OK&&virgin)		 /* any errors?	 did we do anything? */
usg:
   { elog(usage);return EX_USAGE;
   }
  if(invert)
     switch(retval)			 /* we only invert the regular cases */
      { case EX_OK:return EX_CANTCREAT;
	case EX_CANTCREAT:return EX_OK;
      }
  return retval;			       /* all other exitcodes remain */
}

void*tmalloc(len)const size_t len;				     /* stub */
{ return malloc(len);
}

ropen(name,mode,mask)const char*const name;const int mode;const mode_t mask;
{ return open(name,mode,mask);					     /* stub */
}

rclose(fd)const int fd;						     /* stub */
{ return close(fd);
}

void writeerr(a)const char*const a;				     /* stub */
{
}
