/************************************************************************
 *	procmail - The autonomous mail processor			*
 *									*
 *	It has been designed to be able to be run suid root and (in	*
 *	case your mail spool area is *not* world writeable) sgid	*
 *	mail (or daemon), without creating security holes.		*
 *									*
 *	Seems to be perfect.						*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: procmail.c,v 1.60 1994/01/11 13:25:09 berg Exp $";
#endif
#include "../patchlevel.h"
#include "procmail.h"
#include "sublib.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "cstdio.h"
#include "exopen.h"
#include "regexp.h"
#include "goodies.h"
#include "locking.h"
#include "mailfold.h"

static const char fdefault[]="DEFAULT",orgmail[]="ORGMAIL",*const nullp,
 sendmail[]="SENDMAIL",From_[]=FROM,exflags[]=RECFLAGS,drcfile[]="Rcfile:",
 systm_mbox[]=SYSTEM_MBOX,pmusage[]=PM_USAGE,DEFdeflock[]=DEFdefaultlock,
 *etcrc=ETCRC,misrecpt[]="Missing recipient\n";
char*buf,*buf2,*loclock,*tolock;
const char shellflags[]="SHELLFLAGS",shell[]="SHELL",lockfile[]="LOCKFILE",
 shellmetas[]="SHELLMETAS",lockext[]="LOCKEXT",newline[]="\n",binsh[]=BinSh,
 unexpeof[]="Unexpected EOL\n",*const*gargv,*const*restargv= &nullp,*sgetcp,
 *rcfile=PROCMAILRC,dirsep[]=DIRSEP,msgprefix[]="MSGPREFIX",devnull[]=DevNull,
 lgname[]="LOGNAME",executing[]="Executing",oquote[]=" \"",cquote[]="\"\n",
 procmailn[]="procmail",whilstwfor[]=" whilst waiting for ",home[]="HOME",
 maildir[]="MAILDIR",*defdeflock,*argv0="";
char*Stdout;
int retval=EX_CANTCREAT,retvl2=EX_OK,sh,pwait,lcking,rcstate,rc= -1,ignwerr,
 lexitcode=EX_OK,asgnlastf,accspooldir,crestarg,skiprc,savstdout;
size_t linebuf=mx(DEFlinebuf+XTRAlinebuf,STRLEN(systm_mbox)<<1);
volatile int nextexit;			       /* if termination is imminent */
pid_t thepid;
long filled,lastscore;	       /* the length of the mail, and the last score */
char*themail,*thebody;			    /* the head and body of the mail */
uid_t uid;
gid_t gid,sgid;

main(argc,argv)const char*const argv[];
{ register char*chp,*chp2;register i;int suppmunreadable;
  ;{ int Deliverymode,mailfilter;char*fromwhom=0;const char*idhint=0;
#define Presenviron	i
     Deliverymode=mailfilter=0;thepid=getpid();
     if(argc)			       /* sanity check, any argument at all? */
      { Deliverymode=strcmp(lastdirsep(argv0=argv[0]),procmailn);
	for(Presenviron=argc=0;(chp2=(char*)argv[++argc])&&*chp2=='-';)
	   for(;;)				       /* processing options */
	    { switch(*++chp2)
	       { case VERSIONOPT:elog(VERSION);return EX_OK;
		 case HELPOPT1:case HELPOPT2:elog(pmusage);elog(PM_HELP);
		    elog(PM_QREFERENCE);return EX_USAGE;
		 case PRESERVOPT:Presenviron=1;continue;
		 case MAILFILTOPT:mailfilter=1;continue;
		 case TEMPFAILOPT:retval=EX_TEMPFAIL;continue;
		 case FROMWHOPT:case ALTFROMWHOPT:
		    if(*++chp2)
		       fromwhom=chp2;
		    else if(chp2=(char*)argv[argc+1])
		       argc++,fromwhom=chp2;
		    else
		       nlog("Missing name\n");
		    break;
		 case ARGUMENTOPT:
		  { static const char*argv1[]={"",0};
		    if(*++chp2)
		       goto setarg;
		    else if(chp2=(char*)argv[argc+1])
		     { argc++;
setarg:		       *argv1=chp2;restargv=argv1;crestarg=1;
		     }
		    else
		       nlog("Missing argument\n");
		    break;
		  }
		 case DELIVEROPT:
		    if(!*(chp= ++chp2)&&!(chp=(char*)argv[++argc]))
		     { nlog(misrecpt);break;
		     }
		    else
		     { Deliverymode=1;goto last_option;
		     }
		 case '-':
		    if(!*chp2)
		     { argc++;goto last_option;
		     }
		 default:nlog("Unrecognised options:");logqnl(chp2);
		    elog(pmusage);elog("Processing continued\n");
		 case '\0':;
	       }
	      break;
	    }
      }
     if(Deliverymode&&!(chp=chp2))
	nlog(misrecpt),Deliverymode=0;
last_option:
     if(mailfilter)
      { if(Deliverymode)				 /* -d supersedes -m */
	 { mailfilter=0;goto conflopt;
	 }
	if(crestarg)				     /* -m will supersede -a */
conflopt:  nlog("Conflicting options\n"),elog(pmusage);
      }
     if(!Deliverymode)
	idhint=getenv(lgname);
     if(!Presenviron)				     /* drop the environment */
      { const char**emax=(const char**)environ,*const*ep,*const*kp;
	static const char*const keepenv[]=KEEPENV;
	for(kp=keepenv;*kp;kp++)		     /* preserve a happy few */
	   for(i=strlen(*kp),ep=emax;chp2=(char*)*ep;ep++)
	      if(!strncmp(*kp,chp2,i)&&(chp2[i]=='='||chp2[i-1]=='_'))
	       { *emax++=chp2;break;
	       }
	*emax=0;					    /* drop the rest */
      }
#ifdef LD_ENV_FIX
     ;{ const char**emax=(const char**)environ,**ep;
	static const char ld_[]="LD_";
	for(ep=emax;*emax;emax++);	  /* find the end of the environment */
	while(*ep)
	   if(!strncmp(ld_,*ep++,STRLEN(ld_)))	       /* it starts with LD_ */
	      *--ep= *--emax,*emax=0;			/* copy from the end */
      }
#endif /* LD_ENV_FIX */
     ;{ struct passwd*pass,*passinvk,spassinvk;int privs;
	uid_t euid=geteuid();
	if(passinvk=getpwuid(uid=getuid()))	    /* save it by copying it */
	 { spassinvk.pw_uid=passinvk->pw_uid;spassinvk.pw_gid=passinvk->pw_gid;
	   spassinvk.pw_name=tstrdup(passinvk->pw_name);
	   spassinvk.pw_dir=tstrdup(passinvk->pw_dir);
	   spassinvk.pw_shell=tstrdup(passinvk->pw_shell);passinvk= &spassinvk;
	 }
	privs=1;gid=getgid();
	;{ static const char*const trusted_ids[]=TRUSTED_IDS;
	   if(*trusted_ids&&uid!=euid)
	    { struct group*grp;const char*const*kp;
	      if(passinvk)		      /* check out the invoker's uid */
		 for(chp2=passinvk->pw_name,kp=trusted_ids;*kp;)
		    if(!strcmp(chp2,*kp++)) /* is it amongst the privileged? */
		      goto privileged;
	      if(grp=getgrgid(gid))	      /* check out the invoker's gid */
		for(chp2=grp->gr_name,kp=trusted_ids;*kp;)
		   if(!strcmp(chp2,*kp++))    /* is it among the privileged? */
		      goto privileged;
	      privs=0;
	    }
	 }
privileged:				       /* move stdout out of the way */
	endgrent();endpwent();umask(INIT_UMASK);savstdout=rdup(STDOUT);
	fclose(stdout);rclose(STDOUT);			/* just to make sure */
	if(0>opena(devnull))
	 { writeerr(devnull);return EX_OSFILE;	     /* couldn't open stdout */
	 }
#ifdef console
	opnlog(console);
#endif
	setbuf(stdin,(char*)0);buf=malloc(linebuf);buf2=malloc(linebuf);
#ifdef SIGXCPU
	signal(SIGXCPU,SIG_IGN);signal(SIGXFSZ,SIG_IGN);
#endif
#ifdef SIGLOST
	signal(SIGLOST,SIG_IGN);
#endif
#if DEFverbose
	verboff();verbon();
#else
	verbon();verboff();
#endif
	signal(SIGPIPE,SIG_IGN);qsignal(SIGTERM,srequeue);
	qsignal(SIGINT,sbounce);qsignal(SIGHUP,sbounce);
	qsignal(SIGQUIT,slose);signal(SIGALRM,(void(*)())ftimeout);
	ultstr(0,(unsigned long)uid,buf);
	chp2=!passinvk||!*passinvk->pw_name?buf:passinvk->pw_name;
	filled=0;
	;{ const char*fwhom;size_t lfr,linv;int tstamp=0;
	   if(fromwhom&&*fromwhom==REFRESH_TIME&&!fromwhom[1])
	      tstamp=1;
	   fwhom=fromwhom&&!tstamp?fromwhom:chp2;
	   thebody=themail=
	    malloc(2*linebuf+(lfr=strlen(fwhom))+(linv=strlen(chp2)));
	   if(Deliverymode||fromwhom)  /* need to peek for a leading From_ ? */
	    { char*rstart;int r;static const char Fakefield[]=FAKE_FIELD;
	      ;{ time_t t;				 /* the current time */
		 t=time((time_t*)0);strcat(strcpy(buf2,"  "),ctime(&t));
	       }
	      lfr+=STRLEN(From_)+(r=strlen(buf2));
	      if(tstamp)
		 tstamp=r;
	      if(privs)					 /* privileged user? */
		 linv=0;		 /* yes, so no need to insert >From_ */
	      else
		 linv+=STRLEN(Fakefield)+r;
	      while(1==(r=rread(STDIN,themail,1))&&*themail=='\n');
	      i=0;rstart=themail;			     /* skip garbage */
	      if(r>0&&STRLEN(From_)<=(i=rread(	      /* is it a From_ line? */
	       STDIN,rstart+1,linebuf-2-1)+1)&&eqFrom_(themail))
	       { rstart[i]='\0';
		 if(!(rstart=strchr(rstart,'\n')))
		    goto nonewl;		     /* drop long From_ line */
		 i-=++rstart-themail;
		 if(tstamp)
		    lfr=findtstamp(themail+STRLEN(From_),rstart)
		     -themail+tstamp;
		 else if(fromwhom)		       /* discard From_ line */
		  { for(;!(rstart=strchr(themail,'\n'));themail[i]='\0')
nonewl:		       if((i=rread(STDIN,themail,linebuf-2))<=0)
			  break;
		    i=rstart?i-(++rstart-themail):0;
		  }
		 else			       /* leave the From_ line alone */
		  { lfr=0;
		    if(linv)
		       i-=(lfr=rstart-themail);
		  }
	       }
	      else
		 tstamp=0;
	      filled=lfr+linv+i;
	      if(lfr||linv)	     /* move read text beyond our From_ line */
	       { r= *rstart;tmemmove(themail+lfr+linv,rstart,i);
		 if(!linv)
		  { rstart[-tstamp]='\0';
		    if(!tstamp)
		       strcat(strcpy(themail,From_),fwhom);
		  }
		 else
		  { if(lfr)
		     { rstart-=linv;
		       if(tstamp)
			  strcpy(rstart-tstamp,buf2);
		       else
			  strcat(strcat(strcpy(themail,From_),fwhom),buf2);
		     }	       /* insert a >From_ field to distinguish fakes */
		    strcat(strcpy(rstart,Fakefield),chp2);
		  }			  /* overwrite the trailing \0 again */
		 strcat(themail,buf2);themail[lfr+linv]=r;
	       }
	    }
	 }
	readmail(0,0L);			      /* read in the mail completely */
	if(Deliverymode)
	   do			  /* chp should point to the first recipient */
	    { chp2=chp;
#ifndef NO_USER_TO_LOWERCASE_HACK
	      for(;*chp;chp++)		  /* kludge recipient into lowercase */
		 if((unsigned)*chp-'A'<='Z'-'A')
		    *chp+='a'-'A';	 /* getpwnam might be case sensitive */
#endif
	      if(argv[++argc])			  /* more than one recipient */
		 if(pidchild=sfork())
		  { if(forkerr(pidchild,procmailn)||waitfor(pidchild)!=EX_OK)
		       retvl2=retval;
		    pidchild=0;		      /* loop for the next recipient */
		  }
		 else
		  { thepid=getpid();
		    while(argv[++argc]);    /* skip till end of command line */
		  }
	    }
	   while(chp=(char*)argv[argc]);
	gargv=argv+argc;			 /* save it for nextrcfile() */
	if(Deliverymode)
	 { if(!(pass=getpwnam(chp2)))  /* chp2 should point to the recipient */
	    { nlog("Unknown user");logqnl(chp2);return EX_NOUSER;
	    }
	   if(euid==ROOT_uid||
	      passinvk&&passinvk->pw_uid==pass->pw_uid||
	      euid==pass->pw_uid&&!setgid(pass->pw_gid))
	      goto Setuser;
	   nlog("Insufficient privileges\n");
	 }
	if(idhint&&(pass=getpwnam(idhint))&&
	    passinvk&&passinvk->pw_uid==pass->pw_uid||
	   (pass=passinvk))
	  /*
	   *	set preferred uid to the intended recipient
	   */
Setuser: { gid=pass->pw_gid;uid=pass->pw_uid;
	   setdef(lgname,chp= *pass->pw_name?pass->pw_name:buf);
	   setdef(home,pass->pw_dir);
	   if(euid==ROOT_uid)
	      initgroups(chp,gid);
	   endgrent();setdef(shell,*pass->pw_shell?pass->pw_shell:binsh);
	 }
	else		 /* user could not be found, set reasonable defaults */
	  /*
	   *	to prevent security holes, drop any privileges now
	   */
	 { setdef(lgname,buf);setdef(home,RootDir);setdef(shell,binsh);
	   setids();
	 }
	if(passinvk)
	 { free(spassinvk.pw_name);free(spassinvk.pw_dir);
	   free(spassinvk.pw_shell);
	 }
	endpwent();
      }
     setdef(orgmail,systm_mbox);setdef(shellmetas,DEFshellmetas);
     setdef(shellflags,DEFshellflags);setdef(maildir,DEFmaildir);
     setdef(fdefault,DEFdefault);setdef(sendmail,DEFsendmail);
     setdef(lockext,DEFlockext);setdef(msgprefix,DEFmsgprefix);
     ;{ const char*const*kp;static const char*const prestenv[]=PRESTENV;
	for(kp=prestenv;*kp;)	/* preset or wipe some environment variables */
	 { strcpy((char*)(sgetcp=buf2),*kp++);readparse(buf,sgetc,2);
	   sputenv(buf);
	 }
      }				 /* find out the name of our system lockfile */
     sgetcp=DEFdeflock+STRLEN(lockfile)+1;readparse(buf,sgetc,2);
     defdeflock=tstrdup(buf);strcpy(buf,chp=(char*)getenv(orgmail));
     buf[i=lastdirsep(chp)-chp]='\0';
     ;{ struct stat stbuf;			   /* strip off the basename */
	sgid=gid;					/* presumed innocent */
       /*
	*	do we need sgidness to access the mail-spool directory/files?
	*/
	if(!stat(buf,&stbuf))
	 { accspooldir=stbuf.st_mode&(S_IWGRP|S_IWOTH)||uid==stbuf.st_uid;
	   if((uid!=stbuf.st_uid&&
		stbuf.st_gid==getegid()||
	       (rcstate=rc_NOSGID,0))&&
	      (stbuf.st_mode&(S_IWGRP|S_IXGRP|S_IWOTH))==(S_IWGRP|S_IXGRP))
	    { if(!Deliverymode)		     /* we aren't the only deliverer */
		 umask(INIT_UMASK&~S_IRWXG);	  /* make it group-writeable */
	      goto keepgid;
	    }
	   else if(stbuf.st_mode&S_ISGID)
keepgid:      sgid=stbuf.st_gid;   /* keep the gid from the parent directory */
	 }
       /*
	*	check if the default-mailbox-lockfile is owned by the
	*	recipient, if not, mark it for further investigation, it
	*	might need to be removed
	*/
	for(;;)
	 { ;{ int mboxstat;
	      ;{ int goodlock;
		 if(!(goodlock=lstat(defdeflock,&stbuf)||stbuf.st_uid==uid))
		    ultoan((unsigned long)stbuf.st_ino,	  /* i-node numbered */
		     strchr(strcpy(buf+i,BOGUSprefix),'\0'));
		/*
		 *	check if the original/default mailbox of the recipient
		 *	exists, if it does, perform some security checks on it
		 *	(check if it's a regular file, check if it's owned by
		 *	the recipient), if something is wrong try and move the
		 *	bogus mailbox out of the way, create the
		 *	original/default mailbox file, and chown it to
		 *	the recipient
		 */
		 if(lstat(chp,&stbuf))			 /* stat the mailbox */
		  { mboxstat= -(errno==EACCES);goto boglock;
		  }				/* lockfile unrightful owner */
		 mboxstat=1;
		 if(!goodlock&&!(stbuf.st_mode&S_IWGRP))
boglock:	    if(!goodlock)	      /* try & rename bogus lockfile */
		       rename(defdeflock,buf);		   /* out of the way */
	       }
	      if(mboxstat>0||mboxstat<0&&(setids(),!lstat(chp,&stbuf)))
		 if(!(stbuf.st_mode&S_IWUSR)||	     /* recipient can write? */
		    S_ISLNK(stbuf.st_mode)||		/* no symbolic links */
		    (S_ISDIR(stbuf.st_mode)?  /* directories, yes, hardlinks */
		      !(stbuf.st_mode&S_IXUSR):stbuf.st_nlink!=1))     /* no */
		    goto bogusbox;	/* can't deliver to this contraption */
		 else if(stbuf.st_uid!=uid)	      /* recipient not owner */
bogusbox:	  { ultoan((unsigned long)stbuf.st_ino,	  /* i-node numbered */
		     strchr(strcpy(buf+i,BOGUSprefix),'\0'));	    /* bogus */
		    if(rename(chp,buf))	   /* try and move it out of the way */
		       goto fishy;  /* rename failed, something's fishy here */
		  }				/* SysV type autoforwarding? */
		 else if(Deliverymode&&stbuf.st_mode&(S_ISGID|S_ISUID))
		  { nlog("Autoforwarding mailbox found\n");return EX_NOUSER;
		  }
		 else
		    break;			  /* everything is just fine */
	    }
	   if(!xcreat(chp,NORMperm,(time_t*)0,1)) /* can create the mailbox? */
	      break;			      /* yes we could, fine, proceed */
	   if(!lstat(chp,&stbuf))		     /* anything in the way? */
	      continue;			       /* check if it could be valid */
	   setids();					   /* try some magic */
	   if(!xcreat(chp,NORMperm,(time_t*)0,0))		/* try again */
	      break;
	   if(lstat(chp,&stbuf))		      /* nothing in the way? */
fishy:	    { nlog("Couldn't create");logqnl(chp);sputenv(orgmail);
	      sputenv(fdefault);break;				 /* so panic */
	    }
	 }					/* bad news, be conservative */
	umask(INIT_UMASK);
      }
     suppmunreadable=verbose;
     if(!Deliverymode)			       /* not explicit delivery mode */
      { if(*gargv)
	  /*
	   *	really change the uid now, since we are not in explicit
	   *	delivery mode and there are some extra command line arguments
	   */
	   setids();
	if(suppmunreadable=nextrcfile())  /* any rcfile on the command-line? */
	 { etcrc=0;				/* yes, so do not read etcrc */
#ifndef NO_COMSAT
	   if(!getenv(scomsat))
	      setdef(scomsat,DEFcomsat);       /* turn off comsat by default */
#endif
	 }
	else if(mailfilter)
	 { nlog("Missing rcfile\n");return EX_NOINPUT;
	 }
	while(chp=(char*)argv[argc])   /* interpret command line specs first */
	 { argc++;
	   if(!asenvcpy(chp)&&mailfilter)
	    { gargv= &nullp;
	      for(restargv=argv+argc;restargv[crestarg];crestarg++);
	      break;
	    }
	   resettmout();
	 }
      }
   }
  ;{ int succeed,lastcond;struct dyna_long ifstack;
     ifstack.filled=ifstack.tspace=0;ifstack.offs=0;
     if(etcrc)
      { if(0<=bopen(etcrc))
	 { yell(drcfile,etcrc);goto startrc;
	 }
	etcrc=0;
      }
     do					     /* main rcfile interpreter loop */
      { resettmout();
	if(rc<0)					 /* open new rc file */
	 { struct stat stbuf;
	  /*
	   *	if we happen to be still running as root, and the rcfile
	   *	is mounted on a secure NFS-partition, we might not be able
	   *	to access it, so check if we can stat it or don't need any
	   *	sgid privileges, if yes, drop all privs and set uid to
	   *	the recipient beforehand
	   */
	   goto findrc;
	   do
	    { if(suppmunreadable)	  /* should we supress this message? */
fake_rc:	 readerr(buf);
	      if(!nextrcfile())		      /* not available? try the next */
	       { skiprc=0;goto nomore_rc;
	       }
	      suppmunreadable=0;
findrc:	      i=0;		    /* should we keep the current directory? */
	      if(strchr(dirsep,*rcfile)||		   /* absolute path? */
	       *rcfile==chCURDIR&&strchr(dirsep,rcfile[1])&&(i=1)) /* ./ pfx */
		 *buf='\0';		/* do not put anything in front then */
	      else
		 cat(tgetenv(home),MCDIRSEP);	  /* prepend $HOME directory */
	      if(stat(strcat(buf,rcfile),&stbuf)?	      /* accessible? */
	       rcstate==rc_NOSGID:stbuf.st_mode&S_IRUSR)  /* owner-readable? */
		 setids();				/* then transmogrify */
	    }
	   while(0>bopen(buf));			   /* try opening the rcfile */
	   if(i)		  /* opened rcfile in the current directory? */
	    { if(!didchd)
	       { didchd=1;*(chp=strcpy(buf2,maildir)+STRLEN(maildir))='=';
		 *++chp=chCURDIR;*++chp='\0';sputenv(buf2);
	       }
	    }
	   else
	    {/*
	      * OK, so now we have opened an absolute rcfile, but for security
	      * reasons we only accept it if it is owned by the recipient and
	      * is not world writeable, or if the the directory it is in is
	      * not world writeable or does not have the sticky bit set
	      */
	      i= *(chp=lastdirsep(buf));
	      if(lstat(buf,&stbuf)||	      /* /dev/null is a special case */
		 ((stbuf.st_uid!=uid||stbuf.st_mode&S_IWOTH)&&
		   strcmp(devnull,buf)&&(*chp='\0',stat(buf,&stbuf)||
		  (stbuf.st_mode&(S_IWOTH|S_IXOTH))==(S_IWOTH|S_IXOTH)&&
		   !(stbuf.st_mode&S_ISVTX))))
	       { *chp=i;rclose(rc);nlog("Suspicious rcfile\n");goto fake_rc;
	       }
	    }
	  /*
	   *	set uid back to recipient in any case, since we might just
	   *	have opened his/her .procmailrc (don't remove these, since
	   *	the rcfile might have been created after the first stat)
	   */
	   *chp=i;yell(drcfile,buf);setids();firstchd();
startrc:   succeed=lastcond=0;
	 }
	unlock(&loclock);goto commint;		/* unlock any local lockfile */
	do
	 { for(;;)				/* skip the rest of the line */
	    { switch(getb())
	       { default:continue;
		 case '\n':case EOF:;
	       }
	      break;
	    }
commint:   do skipspace();				  /* skip whitespace */
	   while(testb('\n'));
	 }
	while(testb('#'));				   /* no comment :-) */
	if(testb(':'))				       /* check for a recipe */
	 { int locknext;long tobesent;char*startchar;
	   static char flags[maxindex(exflags)];
	   ;{ int nrcond,scored;double score;
	      score=scored=0;
	      readparse(buf,getb,0);
	      ;{ char*chp3;
		 nrcond=strtol(buf,&chp3,10);chp=chp3;
	       }
	      if(chp==buf)				 /* no number parsed */
		 nrcond= -1;
	      if(tolock)	 /* clear temporary buffer for lockfile name */
		 free(tolock);
	      for(i=maxindex(flags);flags[i]=0,i--;);	  /* clear the flags */
	      for(tolock=0,locknext=0;;)
	       { chp=skpspace(chp);
		 switch(i= *chp++)
		  { default:
		       if(!(chp2=strchr(exflags,i)))	    /* a valid flag? */
			{ chp--;break;
			}
		       flags[chp2-exflags]=1;		     /* set the flag */
		    case '\0':
		       if(chp!=Tmnate)		/* if not the real end, skip */
			  continue;
		       break;
		    case ':':locknext=1;    /* yep, local lockfile specified */
		       if(*chp||++chp!=Tmnate)
			  tolock=tstrdup(chp),chp=strchr(chp,'\0')+1;
		  }
		 concatenate(chp);skipped(chp);break;	/* display leftovers */
	       }
	      if(nrcond<0)    /* assume appropriate default nr of conditions */
		 nrcond=!flags[ALSO_NEXT_RECIPE]&&!flags[ALSO_N_IF_SUCC];
	      startchar=themail;tobesent=thebody-themail;
	      if(flags[BODY_GREP])	       /* what needs to be egrepped? */
		 if(flags[HEAD_GREP])
		    tobesent=filled;
		 else
		  { startchar=thebody;tobesent=filled-tobesent;goto noconcat;
		  }
	      if(!skiprc)
		 concon(' ');
noconcat:     i=flags[ALSO_NEXT_RECIPE]?lastcond:1;	  /* init test value */
	      if(flags[ALSO_N_IF_SUCC])
		 i=lastcond&&succeed;	/* only if the last recipe succeeded */
	      if(skiprc)
		 i=0;
	      while(skipspace(),nrcond--,testb('*')||nrcond>=0)
	       { skipspace();getlline(buf2);	    /* any conditions (left) */
		 for(chp=strchr(buf2,'\0');--chp>=buf2;)
		  { switch(*chp)	  /* strip off whitespace at the end */
		     { case ' ':case '\t':*chp='\0';continue;
		     }
		    break;
		  }
		 if(i)				 /* check out all conditions */
		  { int negate,scoreany;double weight,xponent,lscore;
		    negate=scoreany=0;lscore=score;
		    for(chp=buf2+1;;strcpy(buf2,buf))
copydone:	     { switch(*(sgetcp=buf2))
			{ case '0':case '1':case '2':case '3':case '4':
			  case '5':case '6':case '7':case '8':case '9':
			  case '-':case '+':case '.':case ',':
			   { char*chp3;double w;
			     w=stod(buf2,(const char**)&chp3);chp2=chp3;
			     if(chp2>buf2&&*(chp2=skpspace(chp2))=='^')
			      { double x;
				x=stod(chp2+1,(const char**)&chp3);
				if(chp3>chp2+1)
				 { if(score>=MAX32)
				      goto skiptrue;
				   chp2=skpspace(chp3);xponent=x;weight=w;
				   scored=scoreany=1;goto copyrest;
				 }
			      }
			   }
			  default:chp--;     /* no special character, backup */
			  case '\\':
			   { int or_nocase;	/* case-distinction override */
			     static const struct {const char*regkey,*regsubst;}
			      *regsp,regs[]=
			       { {FROMDkey,FROMDsubstitute},
				 {TOkey,TOsubstitute},
				 {FROMMkey,FROMMsubstitute},
				 {0,0}
			       };
			     squeeze(chp);or_nocase=0;goto jinregs;
			     do		   /* find special keyword in regexp */
				if((chp2=strstr(chp,regsp->regkey))&&
				 (chp2==buf2||chp2[-1]!='\\'))	 /* escaped? */
				 { size_t lregs,lregk;		   /* no, so */
				   lregk=strlen(regsp->regkey); /* insert it */
				   tmemmove(
				    chp2+(lregs=strlen(regsp->regsubst)),
				    chp2+lregk,strlen(chp2)-lregk+1);
				   tmemmove(chp2,regsp->regsubst,lregs);
				   if(regsp==regs)	   /* daemon regexp? */
				      or_nocase=1;   /* no case sensitivity! */
jinregs:			   regsp=regs;	/* start over and look again */
				 }
				else
				   regsp++;		     /* next keyword */
			     while(regsp->regkey);
			     ;{ int igncase;
				igncase=or_nocase||!flags[DISTINGUISH_CASE];
				if(scoreany)
				 { struct eps*re;long rest;
				   re=bregcomp(chp,igncase);
				   chp=startchar;rest=tobesent;
				   if(negate)
				    { if(weight&&!bregexec(re,
				       (const uchar*)chp,(const uchar*)chp,
				       (size_t)rest,igncase))
					 score+=weight;
				    }
				   else
				    { double oweight=weight*weight;
				      while(weight&&(chp2=
				       bregexec(re,(const uchar*)startchar,
					(const uchar*)chp,(size_t)rest,
					igncase)))
				       { score+=weight;weight*=xponent;
					 ;{ double nweight;
					    if((nweight=weight*weight)<oweight
					       &&oweight<1)
					       break;
					  }
					 if(chp==chp2)
					    if(score>=MAX32||score<=MIN32)
					       break;
					    else
					       continue;
					 rest-=chp2-chp;chp=chp2;
				       }
				    }
				   free(re);
				 }
				else			     /* egrep for it */
				   i=!!egrepin(chp,startchar,tobesent,
				    !igncase)^negate;
			      }
			     break;
			   }
			  case '$':*buf2='"';squeeze(chp);readparse(buf,sgetc,2);
			     strcpy(buf2,skpspace(buf));goto copydone;
			  case '!':negate^=1;chp2=skpspace(chp);
copyrest:		     strcpy(buf,chp2);continue;
			  case '?':pwait=2;metaparse(chp);inittmout(buf);
			      ignwerr=1;pipin(buf,startchar,tobesent);
			      if(scoreany&&lexitcode>=0)
			       { int j=lexitcode;
				 if(negate)
				    while(--j>=0&&
					  (score+=weight)<MAX32&&
					  score>MIN32)
				       weight*=xponent;
				    else
				       score+=j?xponent:weight;
			       }
			      else if(!!lexitcode^negate)
				 i=0;
			      strcpy(buf2,buf);break;
			  case '>':case '<':readparse(buf,sgetc,2);
			   { long pivot;
			      ;{ char*chp3;
				pivot=strtol(buf+1,&chp3,10);chp=chp3;
			      }
			     skipped(skpspace(chp));strcpy(buf2,buf);
			     if(scoreany)
			      { double f;
				if((*buf=='<')^negate)
				   if(filled)
				      f=(double)pivot/filled;
				   else if(pivot>0)
				      goto plusinfty;
				   else
				      goto mininfty;
				else if(pivot)
				   f=(double)filled/pivot;
				else
				   goto plusinfty;
				score+=weight*tpow(f,xponent);
			      }
			     else if(!((*buf=='<'?
					 filled<pivot:
					 filled>pivot)^
					negate))
				i=0;
			   }
			}
		       break;
		     }
		    if(score>MAX32)		/* chop off at plus infinity */
plusinfty:	       score=MAX32;
		    if(score<=MIN32)	       /* chop off at minus infinity */
mininfty:	       score=MIN32,i=0;
		    if(verbose)	     /* not entirely correct, but it will do */
		     { if(scoreany)
			{ charNUM(num,long);
			  nlog("Score: ");ltstr(7,(long)score,num);elog(num);
			  elog(" ");ltstr(7,(long)(score-lscore),num);
			  elog(num);
			}
		       else
			  nlog(i?"M":"No m"),elog("atch on");
		       if(negate)
			  elog(" !");
		       logqnl(buf2);
		     }
skiptrue:;	  }
	       }
	      lastscore=score;				   /* save it for $= */
	      if(scored&&i&&score<=0)
		 i=0;				     /* it was not a success */
	    }
	   if(!flags[ALSO_NEXT_RECIPE]&&!flags[ALSO_N_IF_SUCC])
	      lastcond=i;		   /* save the outcome for posterity */
	   startchar=themail;tobesent=filled;	    /* body, header or both? */
	   if(flags[PASS_HEAD])
	    { if(!flags[PASS_BODY])
		 tobesent=thebody-themail;
	    }
	   else if(flags[PASS_BODY])
	      tobesent-=(startchar=thebody)-themail;
	   chp=strchr(strcpy(buf,tgetenv(sendmail)),'\0');succeed=sh=0;
	   pwait=flags[WAIT_EXIT]|flags[WAIT_EXIT_QUIET]<<1;
	   ignwerr=flags[IGNORE_WRITERR];Stdout=0;skipspace();
	   if(i)
	      concon('\n');
progrm:	   if(testb('!'))				 /* forward the mail */
	    { if(!i)
		 skiprc++;
	      readparse(chp+1,getb,0);
	      if(i)
	       { if(startchar==themail)
		  { startchar[filled]='\0';		     /* just in case */
		    if(eqFrom_(startchar))    /* leave off any leading From_ */
		       while(i= *startchar++,--tobesent&&i!='\n');
		  }				 /* it confuses some mailers */
		 goto forward;
	       }
	      skiprc--;
	    }
	   else if(testb('|'))				    /* pipe the mail */
	    { getlline(buf2);			 /* get the command to start */
	      if(i)
	       { metaparse(buf2);
		 if(!sh&&buf+1==Tmnate)		      /* just a pipe symbol? */
		  { *buf='|';*(char*)(Tmnate++)='\0';goto tostdout;
		  }						  /* fake it */
forward:	 if(locknext)
		  { if(!tolock)	   /* an explicit lockfile specified already */
		     { *buf2='\0';  /* find the implicit lockfile ('>>name') */
		       for(chp=buf;i= *chp++;)
			  if(i=='>'&&*chp=='>')
			   { chp=skpspace(chp+1);
			     tmemmove(buf2,chp,i=strcspn(chp,EOFName));
			     buf2[i]='\0';
			     if(sh)	 /* expand any environment variables */
			      { chp=tstrdup(buf);sgetcp=buf2;
				readparse(buf,sgetc,0);strcpy(buf2,buf);
				strcpy(buf,chp);free(chp);
			      }
			     break;
			   }
		       if(!*buf2)
			{ nlog("Couldn't determine implicit lockfile from");
			  logqnl(buf);
			}
		     }
		    lcllock();
		    if(!pwait)		/* try and protect the user from his */
		       pwait=2;			   /* blissful ignorance :-) */
		  }
		 inittmout(buf);asgnlastf=1;
		 if(flags[FILTER])
		  { if(startchar==themail&&tobesent!=filled)  /* if only 'h' */
		     { if(!pipthrough(buf,startchar,tobesent))
			  succeed=1,readmail(1,tobesent);
		     }
		    else if(!pipthrough(buf,startchar,tobesent))
		       succeed=1,filled=startchar-themail,readmail(0,0L);
		  }
		 else if(Stdout)		  /* capturing stdout again? */
		  { if(!pipthrough(buf,startchar,tobesent))
		       succeed=1,postStdout();	  /* only parse if no errors */
		  }
		 else if(!pipin(buf,startchar,tobesent)&& /* regular program */
		  (succeed=1,!flags[CONTINUE]))
		    goto frmailed;
	       }
	    }
	   else if(testb(EOF))
	      nlog("Incomplete recipe");
	   else		   /* dump the mail into a mailbox file or directory */
	    { static const char extrns[]="Extraneous ",ignrd[]=" ignored\n";
	      if(flags[FILTER])
		 flags[FILTER]=0,nlog(extrns),elog("filter-flag"),elog(ignrd);
	      if(chp=gobenv(buf))	   /* can it be an environment name? */
	       { if(skipspace())
		    chp++;		   /* keep pace with argument breaks */
		 if(testb('='))		      /* is it really an assignment? */
		  { int c;
		    *chp++='=';*chp='\0';
		    if(skipspace())
		       chp++;
		    ungetb(c=getb());
		    switch(c)
		     { case '!':case '|':		  /* ok, it's a pipe */
			  if(i)
			     primeStdout();
			  goto progrm;
		     }
		  }
	       }		 /* find the end, start of a nesting recipe? */
	      else if((chp=strchr(buf,'\0'))==buf&&testb('{')&&
	       (*chp++='{',*chp='\0',testb(' ')||testb('\t')||testb('\n')))
	       { if(flags[CONTINUE])
		    nlog(extrns),elog("continue-flag"),elog(ignrd);
		 if(locknext)
		    nlog(extrns),elog("locallockfile"),elog(ignrd);
		 app_val(&ifstack,(off_t)lastcond);	    /* push lastcond */
		 if(!i)						/* no match? */
		    skiprc++;		      /* increase the skipping level */
		 continue;
	       }
	      if(!i)						/* no match? */
		 skiprc++;		  /* temporarily disable subprograms */
	      readparse(chp,getb,0);
	      if(i)
tostdout:      { strcpy(buf2,buf);
		 if(locknext)
		    lcllock();		     /* write to a file or directory */
		 inittmout(buf);	  /* to break messed-up kernel locks */
		 if(dump(deliver(buf,strchr(buf,'\0')+1),startchar,tobesent)
		  &&!ignwerr)
		    writeerr(buf);
		 else if(succeed=1,!flags[CONTINUE])
frmailed:	  { if(ifstack.offs)
		       free(ifstack.offs);
		    goto mailed;
		  }
	       }
	      else
		 skiprc--;			     /* reenable subprograms */
	    }
	 }
	else if(testb('}'))					/* end block */
	 { if(skiprc)					    /* just skipping */
	      skiprc--;					   /* decrease level */
	   if(ifstack.filled)		      /* restore lastcond from stack */
	      lastcond=ifstack.offs[--ifstack.filled];
	   else
	      nlog("Closing brace unexpected\n");	      /* stack empty */
	 }
	else				    /* then it must be an assignment */
	 { if(!(chp=gobenv(buf)))
	    { if(!*buf)					/* skip a word first */
		 getbl(buf);				      /* then a line */
	      skipped(buf);continue;			/* display leftovers */
	    }
	   skipspace();
	   if(testb('='))			   /* removal or assignment? */
	      *chp='=',readparse(++chp,getb,1);
	   else
	      *++chp='\0';		     /* throw in a second terminator */
	   if(!skiprc)
	      sputenv(buf),chp[-1]='\0',asenv(chp);
	 }
      }						    /* main interpreter loop */
     while(rc<0||!testb(EOF)||poprc()||etcrc&&(etcrc=0,rclose(rc),rc= -1));
nomore_rc:
     if(ifstack.offs)
	free(ifstack.offs);
   }
  ;{ int succeed;
     concon('\n');succeed=0;
     if(*(chp=(char*)tgetenv(fdefault)))		     /* DEFAULT set? */
      { setuid(uid);firstchd();
	if(strcmp(chp,devnull)&&strcmp(chp,"|"))  /* neither /dev/null nor | */
	   asenvcpy((char*)DEFdeflock);			    /* implicit lock */
	if(dump(deliver(chp,(char*)0),themail,filled))		  /* default */
	   writeerr(buf);
	else
	   succeed=1;
      }
     if(!succeed&&*(chp=(char*)tgetenv(orgmail)))      /* if all else failed */
	if(dump(deliver(chp,(char*)0),themail,filled))	      /* don't panic */
	   writeerr(buf);			      /* try the last resort */
	else
	   succeed=1;
     if(succeed)				     /* should we panic now? */
mailed: retval=EX_OK;			  /* we're home free, mail delivered */
   }
  unlock(&loclock);terminate();
}

eqFrom_(a)const char*const a;
{ return!strncmp(a,From_,STRLEN(From_));
}
