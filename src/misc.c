/************************************************************************
 *	Miscellaneous routines used by procmail				*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: misc.c,v 1.59 1994/08/18 13:45:06 berg Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "sublib.h"
#include "robust.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "cstdio.h"
#include "exopen.h"
#include "regexp.h"
#include "mcommon.h"
#include "goodies.h"
#include "locking.h"
#include "mailfold.h"

struct varval strenvvar[]={{"LOCKSLEEP",DEFlocksleep},
 {"LOCKTIMEOUT",DEFlocktimeout},{"SUSPEND",DEFsuspend},
 {"NORESRETRY",DEFnoresretry},{"TIMEOUT",DEFtimeout},{"VERBOSE",DEFverbose},
 {"LOGABSTRACT",DEFlogabstract}};
struct varstr strenstr[]={{"SHELLMETAS",DEFshellmetas},{"LOCKEXT",DEFlockext},
 {"MSGPREFIX",DEFmsgprefix},{"COMSAT",""},{"TRAP",""},
 {"SHELLFLAGS",DEFshellflags},{"DEFAULT",DEFdefault},{"SENDMAIL",DEFsendmail}};

#define MAXvarvals	 maxindex(strenvvar)
#define MAXvarstrs	 maxindex(strenstr)

const char lastfolder[]="LASTFOLDER",maildir[]="MAILDIR";
int didchd;
char*globlock;
static time_t oldtime;
static int fakedelivery;
		       /* line buffered to keep concurrent entries untangled */
void elog(newt)const char*const newt;
{ int lnew;size_t i;static lold;static char*old;char*p;
#ifndef O_CREAT
  lseek(STDERR,(off_t)0,SEEK_END);	  /* locking should be done actually */
#endif
  if(!(lnew=strlen(newt))||nextexit)			     /* force flush? */
     goto flush;
  i=lold+lnew;
  if(p=lold?realloc(old,i):malloc(i))			 /* unshelled malloc */
   { memmove((old=p)+lold,newt,(size_t)lnew);			   /* append */
     if(p[(lold=i)-1]=='\n')					     /* EOL? */
	rwrite(STDERR,p,(int)i),lold=0,free(p);		/* flush the line(s) */
   }
  else						   /* no memory, force flush */
flush:
   { if(lold)
      { rwrite(STDERR,old,lold);lold=0;
	if(!nextexit)
	   free(old);			/* don't use free in signal handlers */
      }
     if(lnew)
	rwrite(STDERR,newt,lnew);
   }
}

#include "shell.h"

void ignoreterm P((void))
{ signal(SIGTERM,SIG_IGN);signal(SIGHUP,SIG_IGN);signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
}

void shutdesc P((void))
{ rclose(savstdout);closelog();closerc();
}

void setids P((void))
{ if(rcstate!=rc_NORMAL)
   { if(setrgid(gid))	/* due to these !@#$%^&*() POSIX semantics, setgid() */
	setgid(gid);	   /* sets the saved gid as well; we can't use that! */
     setruid(uid);setuid(uid);setegid(gid);rcstate=rc_NORMAL;
#if !DEFverbose
     verbose=0;			/* to avoid peeking in rcfiles using SIGUSR1 */
#endif
   }
}

void writeerr(line)const char*const line;
{ nlog(errwwriting);logqnl(line);
}

int forkerr(pid,a)const pid_t pid;const char*const a;
{ if(pid==-1)
   { nlog("Failed forking");logqnl(a);
     return 1;
   }
  return 0;
}

void progerr(line,xitcode)const char*const line;int xitcode;
{ charNUM(num,thepid);
  nlog("Program failure (");ltstr(0,(long)xitcode,num);elog(num);elog(") of");
  logqnl(line);
}

void chderr(dir)const char*const dir;
{ nlog("Couldn't chdir to");logqnl(dir);
}

void readerr(file)const char*const file;
{ nlog("Couldn't read");logqnl(file);
}

void verboff P((void))
{ verbose=0;
#ifdef SIGUSR1
  qsignal(SIGUSR1,verboff);
#endif
}

void verbon P((void))
{ verbose=1;
#ifdef SIGUSR2
  qsignal(SIGUSR2,verbon);
#endif
}

void yell(a,b)const char*const a,*const b;		/* log if VERBOSE=on */
{ if(verbose)
     nlog(a),logqnl(b);
}

void newid P((void))
{ thepid=getpid();oldtime=0;
}

void zombiecollect P((void))
{ while(waitpid((pid_t)-1,(int*)0,WNOHANG)>0);	      /* collect any zombies */
}

void nlog(a)const char*const a;
{ time_t newtime;
  static const char colnsp[]=": ";
  elog(procmailn);elog(colnsp);
  if(verbose&&oldtime!=(newtime=time((time_t*)0)))
   { charNUM(num,thepid);
     elog("[");oldtime=newtime;ultstr(0,(unsigned long)thepid,num);elog(num);
     elog("] ");elog(ctime(&oldtime));elog(procmailn);elog(colnsp);
   }
  elog(a);
}

void logqnl(a)const char*const a;
{ elog(oquote);elog(a);elog(cquote);
}

void skipped(x)const char*const x;
{ if(*x)
     nlog("Skipped"),logqnl(x);
}

int nextrcfile P((void))	/* next rcfile specified on the command line */
{ const char*p;int rval=2;
  while(p= *gargv)
   { gargv++;
     if(!strchr(p,'='))
      { rcfile=p;
	return rval;
      }
     rval=1;			       /* not the first argument encountered */
   }
  return 0;
}

void onguard P((void))
{ lcking|=lck_LOCKFILE;
}

void offguard P((void))
{ lcking&=~lck_LOCKFILE;
  if(nextexit==1)	  /* make sure we are not inside Terminate() already */
     elog(newline),Terminate();
}

void sterminate P((void))
{ static const char*const msg[]={"memory","fork",	  /* crosscheck with */
   "a file descriptor","a kernel-lock"};	  /* lck_ defs in procmail.h */
  ignoreterm();
  if(pidchild>0)	    /* don't kill what is not ours, we might be root */
     kill(pidchild,SIGTERM);
  if(!nextexit)
   { nextexit=1;nlog("Terminating prematurely");
     if(!(lcking&lck_LOCKFILE))
      { register unsigned i,j;
	if(i=(lcking&~(lck_ALLOCLIB|lck_LOCKFILE))>>1)
	 { elog(whilstwfor);
	   for(j=0;!((i>>=1)&1);j++);
	   elog(msg[j]);
	 }
	elog(newline);Terminate();
      }
   }
}

void Terminate P((void))
{ ignoreterm();
  if(retvl2!=EXIT_SUCCESS)
     fakedelivery=0,retval=retvl2;
  if(getpid()==thepid)
   { if(retval!=EXIT_SUCCESS)
      { tofile=0;lasttell= -1;			  /* mark it for logabstract */
	logabstract(fakedelivery?"**Lost**":
	 retval==EX_TEMPFAIL?"**Requeued**":"**Bounced**");
      }
     else
	logabstract(tgetenv(lastfolder));
     shutdesc();
     if(!(lcking&lck_ALLOCLIB))			/* don't reenter malloc/free */
	exectrap(traps);
     nextexit=2;unlock(&loclock);unlock(&globlock);fdunlock();
   }					/* flush the logfile & exit procmail */
  elog("");exit(fakedelivery==2?EXIT_SUCCESS:retval);
}

void suspend P((void))
{ ssleep((unsigned)suspendv);
}

void app_val(sp,val)struct dyna_long*const sp;const off_t val;
{ if(sp->filled==sp->tspace)			    /* growth limit reached? */
   { if(!sp->offs)
	sp->offs=malloc(1);
     sp->offs=realloc(sp->offs,(sp->tspace+=4)*sizeof sp->offs);   /* expand */
   }
  sp->offs[sp->filled++]=val;				     /* append to it */
}

int alphanum(c)const unsigned c;
{ return numeric(c)||c-'a'<='z'-'a'||c-'A'<='Z'-'A'||c=='_';
}

char*pmrc2buf P((void))
{ sgetcp=pmrc;readparse(buf,sgetc,2);
  return buf;
}

void setmaildir(newdir)const char*const newdir;		    /* destroys buf2 */
{ char*chp;
  didchd=1;*(chp=strcpy(buf2,maildir)+STRLEN(maildir))='=';
  strcpy(++chp,newdir);sputenv(buf2);
}

void srequeue P((void))
{ retval=EX_TEMPFAIL;sterminate();
}

void slose P((void))
{ fakedelivery=2;sterminate();
}

void sbounce P((void))
{ retval=EX_CANTCREAT;sterminate();
}

void catlim(src)register const char*src;
{ register char*dest=buf;register size_t lim=linebuf;
  while(lim&&*dest)
     dest++,lim--;
  if(lim)
   { while(--lim&&(*dest++= *src++));
     *dest='\0';
   }
}

void setdef(name,contents)const char*const name,*const contents;
{ strcat(strcat(strcpy((char*)(sgetcp=buf2),name),"="),contents);
  readparse(buf,sgetc,2);sputenv(buf);
}

void metaparse(p)const char*p;				    /* result in buf */
{ if(sh=!!strpbrk(p,shellmetas))
     strcpy(buf,p);			 /* copy literally, shell will parse */
  else
#ifndef GOT_bin_test
   { sgetcp=p=tstrdup(p);
     readparse(buf,sgetc,0);				/* parse it yourself */
     if(!strcmp(test,buf))
	strcpy(buf,p),sh=1;			       /* oops, `test' found */
     free((char*)p);
   }
#else
     sgetcp=p,readparse(buf,sgetc,0);
#endif
}

void concatenate(p)register char*p;
{ while(p!=Tmnate)			  /* concatenate all other arguments */
   { while(*p++);
     p[-1]=' ';
   }
  *p=p[-1]='\0';
}

char*cat(a,b)const char*const a,*const b;
{ return strcat(strcpy(buf,a),b);
}

char*tstrdup(a)const char*const a;
{ int i;
  i=strlen(a)+1;
  return tmemmove(malloc(i),a,i);
}

const char*tgetenv(a)const char*const a;
{ const char*b;
  return (b=getenv(a))?b:"";
}

char*cstr(a,b)char*const a;const char*const b;	/* dynamic buffer management */
{ if(a)
     free(a);
  return tstrdup(b);
}

void setlastfolder(folder)const char*const folder;
{ if(asgnlastf)
   { char*chp;
     asgnlastf=0;
     strcpy(chp=malloc(STRLEN(lastfolder)+1+strlen(folder)+1),lastfolder);
     chp[STRLEN(lastfolder)]='=';strcpy(chp+STRLEN(lastfolder)+1,folder);
     sputenv(chp);free(chp);
   }
}

char*gobenv(chp)char*chp;
{ int found,i;
  found=0;
  if(alphanum(i=getb())&&!numeric(i))
     for(found=1;*chp++=i,alphanum(i=getb()););
  *chp='\0';ungetb(i);
  switch(i)
   { case ' ':case '\t':case '\n':case '=':
	if(found)
	   return chp;
   }
  return 0;
}

int asenvcpy(src)char*src;
{ strcpy(buf,src);
  if(src=strchr(buf,'='))			     /* is it an assignment? */
   { const char*chp;
     strcpy((char*)(sgetcp=buf2),++src);readparse(src,sgetc,2);
     chp=sputenv(buf);src[-1]='\0';asenv(chp);
     return 1;
   }
  return 0;
}

void asenv(chp)const char*const chp;
{ static const char slinebuf[]="LINEBUF",logfile[]="LOGFILE",Log[]="LOG",
   sdelivered[]="DELIVERED",includerc[]="INCLUDERC",eumask[]="UMASK",
   dropprivs[]="DROPPRIVS",shift[]="SHIFT";
  if(!strcmp(buf,slinebuf))
   { if((linebuf=renvint(0L,chp)+XTRAlinebuf)<MINlinebuf+XTRAlinebuf)
	linebuf=MINlinebuf+XTRAlinebuf;		       /* check minimum size */
     free(buf);free(buf2);buf=malloc(linebuf);buf2=malloc(linebuf);
   }
  else if(!strcmp(buf,maildir))
     if(chdir(chp))
	chderr(chp);
     else
	didchd=1;
  else if(!strcmp(buf,logfile))
     opnlog(chp);
  else if(!strcmp(buf,Log))
     elog(chp);
  else if(!strcmp(buf,exitcode))
     setxit=1;
  else if(!strcmp(buf,shift))
   { int i;
     if((i=renvint(0L,chp))>0)
      { if(i>crestarg)
	   i=crestarg;
	crestarg-=i;restargv+=i;		     /* shift away arguments */
      }
   }
  else if(!strcmp(buf,dropprivs))			  /* drop privileges */
   { if(renvint(0L,chp))
      { if(verbose)
	   nlog("Assuming identity of the recipient, VERBOSE=off\n");
	setids();
      }
   }
  else if(!strcmp(buf,sdelivered))			    /* fake delivery */
   { if(renvint(0L,chp))				    /* is it really? */
      { onguard();
	if((thepid=sfork())>0)			/* signals may cause trouble */
	   nextexit=2,lcking&=~lck_LOCKFILE,exit(retvl2);
	if(!forkerr(thepid,procmailn))
	   fakedelivery=1;
	newid();offguard();
      }
   }
  else if(!strcmp(buf,lockfile))
     lockit((char*)chp,&globlock);
  else if(!strcmp(buf,eumask))
     doumask((mode_t)strtol(chp,(char**)0,8));
  else if(!strcmp(buf,includerc))
     pushrc(chp);
  else if(!strcmp(buf,host))
   { const char*name;
     if(strcmp(chp,name=hostname()))
      { yell("HOST mismatched",name);
	if(rc<0||!nextrcfile())			  /* if no rcfile opened yet */
	   retval=EXIT_SUCCESS,Terminate();	  /* exit gracefully as well */
	closerc();
      }
   }
  else
   { int i=MAXvarvals;
     do					      /* several numeric assignments */
	if(!strcmp(buf,strenvvar[i].name))
	   strenvvar[i].val=renvint(strenvvar[i].val,chp);
     while(i--);
     i=MAXvarstrs;
     do						 /* several text assignments */
	if(!strcmp(buf,strenstr[i].sname))
	   strenstr[i].sval=chp;
     while(i--);
   }
}

long renvint(i,env)const long i;const char*const env;
{ const char*p;long t;
  t=strtol(env,(char**)&p,10);			  /* parse like a decimal nr */
  if(p==env)
   { for(;;p++)					  /* skip leading whitespace */
      { switch(*p)
	 { case '\t':case ' ':
	      continue;
	 }
	break;
      }
     t=i;
     if(!strnIcmp(p,"on",(size_t)2)||!strnIcmp(p,"y",(size_t)1)||
      !strnIcmp(p,"t",(size_t)1)||!strnIcmp(p,"e",(size_t)1))
	t=1;
     else if(!strnIcmp(p,"off",(size_t)3)||!strnIcmp(p,"n",(size_t)1)||
      !strnIcmp(p,"f",(size_t)1)||!strnIcmp(p,"d",(size_t)1))
	t=0;
   }
  return t;
}

void squeeze(target)char*target;
{ int state;char*src;
  for(state=0,src=target;;target++,src++)
   { switch(*target= *src)
      { case '\n':
	   if(state==1)
	      target-=2;			     /* throw out \ \n pairs */
	   state=2;
	   continue;
	case '\\':state=1;
	   continue;
	case ' ':case '\t':
	   if(state==2)					     /* skip leading */
	    { target--;					       /* whitespace */
	      continue;
	    }
	default:state=0;
	   continue;
	case '\0':;
      }
     break;
   }
}

char*egrepin(expr,source,len,casesens)char*expr;const char*source;
 const long len;int casesens;
{ if(*expr)		 /* only do the search if the expression is nonempty */
   { source=(const char*)bregexec((struct eps*)(expr=(char*)
      bregcomp(expr,!casesens)),(const uchar*)source,(const uchar*)source,
      len>0?(size_t)len:(size_t)0,!casesens);
     free(expr);
   }
  return (char*)source;
}

const struct passwd*savepass(spass,uid)struct passwd*const spass;
 const uid_t uid;
{ struct passwd*tpass;
  if(spass->pw_name&&spass->pw_uid==uid)
     goto ret;
  if(tpass=getpwuid(uid))				  /* save by copying */
   { spass->pw_uid=tpass->pw_uid;spass->pw_gid=tpass->pw_gid;
     spass->pw_name=cstr(spass->pw_name,tpass->pw_name);
     spass->pw_dir=cstr(spass->pw_dir,tpass->pw_dir);
     spass->pw_shell=cstr(spass->pw_shell,tpass->pw_shell);
ret: return spass;
   }
  return (const struct passwd*)0;
}

int enoughprivs(passinvk,euid,egid,uid,gid)const struct passwd*const passinvk;
 const uid_t euid,uid;const gid_t egid,gid;
{ return euid==ROOT_uid||passinvk&&passinvk->pw_uid==uid||euid==uid&&egid==gid;
}

void initdefenv P((void))
{ int i=MAXvarstrs;
  do	   /* initialise all non-empty string variables into the environment */
     if(*strenstr[i].sval)
	setdef(strenstr[i].sname,strenstr[i].sval);
  while(i--);
}

const char*newdynstring(adrp,chp)struct dynstring**const adrp;
 const char*const chp;
{ struct dynstring*curr;size_t len;
  curr=malloc(ioffsetof(struct dynstring,ename[0])+(len=strlen(chp)+1));
  tmemmove(curr->ename,chp,len);curr->enext= *adrp;*adrp=curr;
  return curr->ename;
}
