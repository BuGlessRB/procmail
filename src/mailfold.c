/************************************************************************
 *	Routines that deal with the mailfolder(format)			*
 *									*
 *	Copyright (c) 1990-1999, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: mailfold.c,v 1.90 1999/11/04 23:26:18 guenther Exp $";
#endif
#include "procmail.h"
#include "acommon.h"
#include "sublib.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "exopen.h"
#include "goodies.h"
#include "locking.h"
#include "lastdirsep.h"
#include "mailfold.h"

int logopened,tofile,rawnonl;
off_t lasttell;
static long lastdump;
static volatile int mailread;	/* if the mail is completely read in already */
static struct dyna_long confield;		  /* escapes, concatenations */
static const char*realstart,*restbody;
static const char from_expr[]=FROM_EXPR;

static const char*fifrom(fromw,lbound,ubound)
 const char*fromw,*const lbound;char*const ubound;
{ int i;					   /* terminate & scan block */
  i= *ubound;*ubound='\0';fromw=strstr(mx(fromw,lbound),from_expr);*ubound=i;
  return fromw;
}

static int doesc;
			       /* inserts escape characters on outgoing mail */
static long getchunk(s,fromw,len)const int s;const char*fromw;const long len;
{ static const char esc[]=ESCAP,*ffrom,*endp;
  if(doesc)		       /* still something to escape since last time? */
     doesc=0,rwrite(s,esc,STRLEN(esc)),lastdump++;		/* escape it */
  ffrom=0;					 /* start with a clean slate */
  if(fromw<thebody)			   /* are we writing the header now? */
     ffrom=fifrom(fromw,realstart,thebody);		      /* scan header */
  if(!ffrom&&(endp=fromw+len)>restbody)	       /* nothing yet? but in range? */
   { if((endp+=STRLEN(from_expr)-1)>(ffrom=themail+filled))	/* add slack */
	endp=(char*)ffrom;		  /* make sure we stay within bounds */
     ffrom=fifrom(fromw,restbody,endp);			  /* scan body block */
   }
  return ffrom?(doesc=1,(ffrom-fromw)+1L):len;	 /* +1 to write out the '\n' */
}

long dump(s,source,len)const int s;const char*source;long len;
{ int i;long part;
  lasttell=i= -1;SETerrno(EBADF);
  if(s>=0)
   { if(to_lock(tofile)&&(lseek(s,(off_t)0,SEEK_END),fdlock(s)))
	nlog("Kernel-lock failed\n");
     lastdump=len;doesc=0;
     part=to_delim(tofile)&&!rawnonl?getchunk(s,source,len):len;
     lasttell=lseek(s,(off_t)0,SEEK_END);
     if(!rawnonl)
      { smboxseparator(s);			       /* optional separator */
#ifndef NO_NFS_ATIME_HACK	       /* if it is a file, trick NFS into an */
	if(part&&to_atime(tofile))			    /* a_time<m_time */
	 { struct stat stbuf;
	   rwrite(s,source++,1);len--;part--;		     /* set the trap */
	   if(fstat(s,&stbuf)||					  /* needed? */
	    stbuf.st_mtime==stbuf.st_atime)
	      ssleep(1);  /* ...what a difference this (tea) second makes... */
	 }
#endif
      }
     goto jin;
     do
      { part=getchunk(s,source,len);
jin:	while(part&&(i=rwrite(s,source,BLKSIZ<part?BLKSIZ:(int)part)))
	 { if(i<0)
	      goto writefin;
	   part-=i;len-=i;source+=i;
	 }
      }
     while(len);
     if(!rawnonl)
      { if(!len&&(lastdump<2||!(source[-1]=='\n'&&source[-2]=='\n')))
	   lastdump++,rwrite(s,newline,1);     /* message always ends with a */
	emboxseparator(s);	 /* newline and an optional custom separator */
      }
writefin:
     i=tofile&&fsync(s)&&errno!=EINVAL;		  /* EINVAL => wasn't a file */
     if(to_lock(tofile))
      { int serrno=errno;		       /* save any error information */
	if(fdunlock())
	   nlog("Kernel-unlock failed\n");
	SETerrno(serrno);
      }
     i=rclose(s)||i;
   }			   /* return an error even if nothing was to be sent */
  tofile=0;
  return i&&!len?-1:len;
}

static const char
 maildirtmp[]=MAILDIRtmp,maildircur[]=MAILDIRcur,maildirnew[]=MAILDIRnew;
#define MAILDIRLEN STRLEN(maildirtmp)

static int dirfile(chp,linkonly,tofile)char*const chp;const int linkonly,tofile;
{ static const char lkingto[]="Linking to";
  if(tofile==to_MH)
   { long i=0;			     /* first let us try to prime i with the */
#ifndef NOopendir		     /* highest MH folder number we can find */
     long j;DIR*dirp;struct dirent*dp;char*chp2;
     if(dirp=opendir(buf))
      { while(dp=readdir(dirp))		/* there still are directory entries */
	   if((j=strtol(dp->d_name,&chp2,10))>i&&!*chp2)
	      i=j;			    /* yep, we found a higher number */
	closedir(dirp);				     /* aren't we neat today */
      }
     else
	readerr(buf);
#endif /* NOopendir */
     if(chp-buf+sizeNUM(i)-XTRAlinebuf>linebuf)
exlb: { nlog(exceededlb);setoverflow();
	goto ret;
      }
     ;{ int ok;
	do ultstr(0,++i,chp);		       /* find first empty MH folder */
	while((ok=linkonly?rlink(buf2,buf,0):hlink(buf2,buf))&&errno==EEXIST);
	if(linkonly)
	 { yell(lkingto,buf);
	   if(ok)
	      goto nolnk;
	   goto didlnk;
	 }
      }
     unlink(buf2);
     goto opn;
   }
  else if(tofile==to_MAILDIR)
   { if(!unique(buf,strcpy(chp+MAILDIRLEN,MCDIRSEP_)+1,linebuf,NORMperm,
      verbose,0))
	goto ret;
     unlink(buf);			 /* found a name, remove file in tmp */
     strncpy(chp,maildirnew,MAILDIRLEN);    /* but to link directly into new */
   }
  else								   /* to_DIR */
   { struct stat stbuf;
     size_t mpl=strlen(msgprefix);
     if(chp-buf+mpl+sizeNUM(stbuf.st_ino)-XTRAlinebuf>linebuf)
	goto exlb;
     stat(buf2,&stbuf);			      /* filename with i-node number */
     ultoan((unsigned long)stbuf.st_ino,strcpy(chp,msgprefix)+mpl);
     if(!linkonly&&(!stat(buf,&stbuf)||errno!=ENOENT))
	goto ret;			 /* avoid overwriting an old message */
   }
  if(linkonly)
   { yell(lkingto,buf);
     if(rlink(buf2,buf,0)) /* hardlink the new file, it's a directory folder */
nolnk:	nlog("Couldn't make link to"),logqnl(buf);
     else
didlnk:
      { size_t len;char*p;
	Stdout=buf;primeStdout(empty);
	len=Stdfilled+strlen(Stdout+Stdfilled);
	p=realloc(Stdout,(Stdfilled=len+1+strlen(buf))+1);
	p[len]=' ';strcpy(p+len+1,buf);retbStdout(p);
      }
     goto ret;
   }
  if(!rename(buf2,buf))		       /* rename it, we need the same i-node */
opn: return opena(buf);
ret:
  return -1;
}

int rnmbogus(name,stbuf,i,dolog)const char*const name;	      /* move a file */
 const struct stat*const stbuf;const int i,dolog;	   /* out of the way */
{ static const char renbogus[]="Renamed bogus \"%s\" into \"%s\"",
   renfbogus[]="Couldn't rename bogus \"%s\" into \"%s\"",
   bogusprefix[]=BOGUSprefix;char*p;
  p=strchr(strcpy(strcpy(buf2+i,bogusprefix)+STRLEN(bogusprefix),
   getenv(lgname)),'\0');
  *p++='.';ultoan((unsigned long)stbuf->st_ino,p);	  /* i-node numbered */
  if(dolog)
   { nlog("Renaming bogus mailbox \"");elog(name);elog("\" info");
     logqnl(buf2);
   }
  if(rename(name,buf2))			   /* try and move it out of the way */
   { syslog(LOG_ALERT,renfbogus,name,buf2);		 /* danger!  danger! */
     return 1;
   }
  syslog(LOG_CRIT,renbogus,name,buf2);
  return 0;
}

/* return the named object's mode, making it a directory if it doesn't exist
 * and renaming it out of the way if it's not _just_right_ and we're being
 * paranoid */
static mode_t trymkdir(dir,paranoid,i)const char*const dir;const int paranoid,i;
{ int(*dostat)P((const char*,struct stat*));struct stat stbuf;int tries;
  dostat=paranoid?&lstat:&stat;tries=3-1;    /* minus one for post-decrement */
  do
   { if(!dostat(dir,&stbuf))		      /* does it exist?	 Is it okay? */
      { if(!paranoid||				  /* if we're trusting it is */
	   (S_ISDIR(stbuf.st_mode)&&	      /* else it must be a directory */
	    (stbuf.st_uid==uid||	       /* and have the correct owner */
	     !stbuf.st_uid&&!chown(dir,uid,sgid))))  /* or be safely fixable */
	   return stbuf.st_mode;				   /* bingo! */
	else if(rnmbogus(dir,&stbuf,i,1))  /* try and move it out of the way */
	   break;						/* couldn't! */
      }
     else if(errno!=ENOENT)	    /* something more fundamental went wrong */
	break;
     else if(!mkdir(dir,NORMdirperm))	  /* it's not there, can we make it? */
      { if(!paranoid)	      /* do we need to double check the permissions? */
	   return S_IFDIR|NORMdirperm&~cumask;			     /* nope */
	tries++;		/* the mkdir succeeded, so take another shot */
      }
   }while(tries-->0);
  return 0;
}

static int mkmaildir(chp,paranoid)char*chp;const int paranoid;
{ mode_t mode;int i;
  if(paranoid)
     strncpy(buf2,buf,i=chp-buf+1),buf2[i-1]=*MCDIRSEP_,buf2[i]='\0';
  return
   (strcpy(chp,maildirnew),mode=trymkdir(buf,paranoid,i),S_ISDIR(mode))&&
   (strcpy(chp,maildircur),mode=trymkdir(buf,paranoid,i),S_ISDIR(mode))&&
   (strcpy(chp,maildirtmp),mode=trymkdir(buf,paranoid,i),S_ISDIR(mode));
}					      /* leave tmp in buf on success */

int foldertype(chp,modep,forcedir,paranoid)char*chp;mode_t*const modep;
 int forcedir;struct stat*const paranoid;
{ struct stat stbuf;mode_t mode;int type,i;
  int(*dostat)(const char*,struct stat*);
  i=0;dostat=paranoid?&lstat:&stat;
  if(chp>=buf+1&&chp[-1]==*MCDIRSEP_&&*chp==chCURDIR)
   { *--chp='\0';type=to_MH;
   }
  else if(*chp==*MCDIRSEP_&&chp>buf)
   { *chp='\0';type=to_MAILDIR;
     i=MAILDIRLEN;
   }
  else
   { chp++;					    /* resolve the ambiguity */
     if(!forcedir)
      { if(dostat(buf,&stbuf))
	 { if(paranoid)
	    { type=to_NOTYET;
	      goto ret;
	    }
	   goto newfile;
	 }
	else if(mode=stbuf.st_mode,!S_ISDIR(mode))
	   goto file;
      }
     type=to_DIR;
   }
  if((chp-buf)+UNIQnamelen+1+i>linebuf)
   { type=to_TOOLONG;
     goto ret;
   }
  if(type==to_DIR&&!forcedir)		  /* we've already checked this case */
     goto done;
  if(paranoid)
     strncpy(buf2,buf,i=lastdirsep(buf)-buf),buf2[i]='\0';
  mode=trymkdir(buf,paranoid!=0,i);
  if(!S_ISDIR(mode)||(type==to_MAILDIR&&
   (forcedir=1,!mkmaildir(chp,paranoid!=0))))
   { nlog("Unable to treat as directory");logqnl(buf);	 /* we can't make it */
     if(forcedir)				     /* fallback or give up? */
      { *chp='\0';skipped(buf);type=to_CANTCREATE;
	goto ret;
      }
     if(!mode)
newfile:mode=S_IFREG|NORMperm&~cumask;
file:type=to_FILE;
   }
done:
  if(paranoid)
     *paranoid=stbuf;
  else
     *modep=mode;
ret:
  return type;
}

int writefolder(boxname,linkfolder,source,len,ignwerr,dolock)
 char*boxname,*linkfolder;const char*source;long len;const int ignwerr,dolock;
{ char*chp,*chp2;mode_t mode;int fd;
  asgnlastf=1;
  if(*boxname=='|'&&(!linkfolder||linkfolder==Tmnate))
   { setlastfolder(boxname);
     fd=rdup(savstdout);
     goto dump;
   }
  if(boxname!=buf)
     strcpy(buf,boxname);		 /* boxname can be found back in buf */
  if(*(chp=buf))				  /* not null length target? */
     chp=strchr(buf,'\0')-1;		     /* point to just before the end */
  if(linkfolder)		    /* any additional directories specified? */
   { size_t blen;
     if(blen=Tmnate-linkfolder)		       /* copy the names into safety */
	Tmnate=(linkfolder=tmemmove(malloc(blen),linkfolder,blen))+blen;
     else
	linkfolder=0;
   }
/*tofile=foldertype(chp,&mode,linkfolder!=0,0)			perhaps? XXX */
  switch(tofile=foldertype(chp,&mode,0,0))	     /* the envelope please! */
   { case to_FILE:
	if(linkfolder)	  /* any leftovers?  Now is the time to display them */
	   concatenate(linkfolder),skipped(linkfolder),free(linkfolder);
	if(!strcmp(devnull,buf))
	   tofile=0,rawnonl=1;		     /* save the effort on /dev/null */
	else if(!(UPDATE_MASK&(mode|cumask)))
	   chmod(boxname,mode|UPDATE_MASK);
	if(dolock&&tofile)
	 { strcpy(chp=strchr(buf,'\0'),lockext);
	   if(!globlock||strcmp(buf,globlock))
	      lockit(tstrdup(buf),&loclock);
	   *chp='\0';
	 }
	fd=opena(boxname);
dump:	if(dump(fd,source,len)&&!ignwerr)
dumpf:	 { switch(errno)
	    { case ENOSPC:nlog("No space left to finish writing"),logqnl(buf);
		 break;
#ifdef EDQUOT
	      case EDQUOT:nlog("Quota exceeded while writing"),logqnl(buf);
		 break;
#endif
	      default:writeerr(buf);
	    }
	   if(lasttell>=0&&!truncate(boxname,lasttell)&&(logopened||verbose))
	      nlog("Truncated file to former size\n");	    /* undo garbage */
ret0:	   return 0;
	 }
	return 1;
     case to_TOOLONG:
exlb:	nlog(exceededlb);setoverflow();
     case to_CANTCREATE:
retf:	if(linkfolder)
	   free(linkfolder);
	goto ret0;
     case to_MAILDIR:
	chp2=buf2+(chp-buf);
	strcpy(buf2,buf);
	/* chp2=stpcpy(buf2,buf)-MAILDIRLEN; */
	strcpy(chp+=MAILDIRLEN,MCDIRSEP_);
	if(0>(fd=unique(buf,++chp,linebuf,NORMperm,verbose,doFD)))
	   goto nfail;
	if(source==themail)			      /* skip leading From_? */
	   source=skipFrom_(source,&len);
	if(dump(fd,source,len)&&!ignwerr)
	   goto failed;
	strcpy(strcpy(strcpy(chp2,maildirnew)+MAILDIRLEN,MCDIRSEP_)+1,chp);
	if(rename(buf,buf2))
	 { unlink(buf);
nfail:	   nlog("Couldn't create or rename temp file");logqnl(buf);
	   goto retf;
	 }
	setlastfolder(buf2);
	break;
     case to_DIR:
	chp+=2;
     default: /* case to_MH: */
	strcpy(chp-1,MCDIRSEP_);
#if 0
	if(tofile==to_MH&&source==themail)
	   source=skipFrom_(source,&len);			      /* XXX */
#endif
	chp2=buf2+(chp-buf);
	strcpy(buf2,buf);
	/* chp2=stpcpy(buf2,buf); */
	if(!unique(buf2,chp2,linebuf,NORMperm,verbose,0)||
	 0>(fd=dirfile(chp,0,tofile)))
	   goto nfail;
	if(dump(fd,source,len)&&!ignwerr)
	 { strcpy(buf,buf2);
failed:	   unlink(buf);lasttell= -1;
	   if(linkfolder)
	      free(linkfolder);
	   goto dumpf;
	 }
	strcpy(buf2,buf);
	break;
   }
  if(!(UPDATE_MASK&(mode|cumask)))
   { chp[-1]='\0';				      /* restore folder name */
     chmod(buf,mode|UPDATE_MASK);
   }
  if(linkfolder)
   { for(boxname=linkfolder;boxname!=Tmnate;boxname=strchr(boxname,'\0')+1)
      { strcpy(buf,boxname);
	if(*(chp=buf))
	   chp=strchr(buf,'\0')-1;
	switch(tofile=foldertype(chp,&mode,1,0))
	 { case to_TOOLONG:goto exlb;
	   case to_CANTCREATE:				     /* just skip it */
	      continue;
	   case to_DIR:
	      chp+=2;
	   case to_MH:
	      strcpy(chp-1,MCDIRSEP_);
	   case to_MAILDIR:
	      break;
	 }
	if(dirfile(chp,1,tofile))	/* link it with the original in buf2 */
	   if(!(UPDATE_MASK&(mode|cumask)))
	    { chp[-1]='\0';
	      chmod(buf,mode|UPDATE_MASK);
	    }
      }
     free(linkfolder);
   }
  return 1;
}

void logabstract(lstfolder)const char*const lstfolder;
{ if(lgabstract>0||(logopened||verbose)&&lgabstract)  /* don't mail it back? */
   { char*chp,*chp2;int i;static const char sfolder[]=FOLDER;
     if(mailread)			  /* is the mail completely read in? */
      { i= *thebody;*thebody='\0';     /* terminate the header, just in case */
	if(eqFrom_(chp=themail))		       /* any "From " header */
	 { if(chp=strchr(themail,'\n'))
	      *chp='\0';
	   else
	      chp=thebody;			  /* preserve mailbox format */
	   elog(themail);elog(newline);*chp='\n';	     /* (any length) */
	 }
	*thebody=i;			   /* eliminate the terminator again */
	if(!(lcking&lck_ALLOCLIB)&&		/* don't reenter malloc/free */
	 (chp=egrepin(NSUBJECT,chp,(long)(thebody-chp),0)))
	 { size_t subjlen;
	   for(chp2= --chp;*--chp2!='\n';);
	   if((subjlen=chp-++chp2)>MAXSUBJECTSHOW)
	      subjlen=MAXSUBJECTSHOW;		    /* keep it within bounds */
	   ((char*)tmemmove(buf,chp2,subjlen))[subjlen]='\0';detab(buf);
	   elog(" ");elog(buf);elog(newline);
	 }
      }
     elog(sfolder);i=strlen(strncpy(buf,lstfolder,MAXfoldlen))+STRLEN(sfolder);
     buf[MAXfoldlen]='\0';detab(buf);elog(buf);i-=i%TABWIDTH;	/* last dump */
     do elog(TABCHAR);
     while((i+=TABWIDTH)<LENoffset);
     ultstr(7,lastdump,buf);elog(buf);elog(newline);
   }
}

static int concnd;				 /* last concatenation value */

void concon(ch)const int ch;   /* flip between concatenated and split fields */
{ size_t i;
  if(concnd!=ch)				   /* is this run redundant? */
   { concnd=ch;			      /* no, but note this one for next time */
     for(i=confield.filled;i;)		   /* step through the saved offsets */
	themail[acc_vall(confield,--i)]=ch;	       /* and flip every one */
   }
}

void readmail(rhead,tobesent)const long tobesent;
{ char*chp,*pastend;static size_t contlengthoffset;
  ;{ long dfilled;
     if(rhead)					/* only read in a new header */
      { dfilled=mailread=0;chp=readdyn(malloc(1),&dfilled);filled-=tobesent;
	if(tobesent<dfilled)		   /* adjust buffer size (grow only) */
	 { char*oldp=themail;
	   thebody=(themail=realloc(themail,dfilled+filled))+(thebody-oldp);
	 }
	tmemmove(themail+dfilled,thebody,filled);tmemmove(themail,chp,dfilled);
	free(chp);themail=realloc(themail,1+(filled+=dfilled));
      }
     else
      { if(!mailread||!filled)
	   rhead=1;	 /* yup, we read in a new header as well as new mail */
	mailread=0;dfilled=thebody-themail;themail=readdyn(themail,&filled);
      }
     *(pastend=filled+(thebody=themail))='\0';		   /* terminate mail */
     while(thebody<pastend&&*thebody++=='\n');	     /* skip leading garbage */
     realstart=thebody;
     if(rhead)			      /* did we read in a new header anyway? */
      { confield.filled=0;concnd='\n';
	while(thebody=strchr(thebody,'\n'))
	   switch(*++thebody)			    /* mark continued fields */
	    { case '\t':case ' ':app_vall(confield,(long)(thebody-1-themail));
	      default:
		 continue;		   /* empty line marks end of header */
	      case '\n':thebody++;
		 goto eofheader;
	    }
	thebody=pastend;      /* provide a default, in case there is no body */
eofheader:
	contlengthoffset=0;		      /* traditional Berkeley format */
	if(!berkeley&&				  /* ignores Content-Length: */
	   (chp=egrepin("^Content-Length:",themail,(long)(thebody-themail),0)))
	   contlengthoffset=chp-themail;
      }
     else			       /* no new header read, keep it simple */
	thebody=themail+dfilled; /* that means we know where the body starts */
   }		      /* to make sure that the first From_ line is uninjured */
  if((chp=thebody)>themail)
     chp--;
  if(contlengthoffset)
   { unsigned places;long cntlen,actcntlen;charNUM(num,cntlen);
     chp=themail+contlengthoffset;cntlen=filled-(thebody-themail);
     if(filled>1&&themail[filled-2]=='\n')		 /* no phantom '\n'? */
	cntlen--;		     /* make sure it points to the last '\n' */
     for(actcntlen=places=0;;)
      { switch(*chp)
	 { default:					/* fill'r up, please */
	      if(places<=sizeof num-2)
		 *chp++='9',places++,actcntlen=(unsigned long)actcntlen*10+9;
	      else
		 *chp++=' ';		 /* ultra long Content-Length: field */
	      continue;
	   case '\n':case '\0':;		      /* ok, end of the line */
	 }
	break;
      }
     if(cntlen<=0)			       /* any Content-Length at all? */
	cntlen=0;
     ultstr(places,cntlen,num);			       /* our preferred size */
     if(!num[places])		       /* does it fit in the existing space? */
	tmemmove(themail+contlengthoffset,num,places),actcntlen=cntlen;
     chp=thebody+actcntlen;		  /* skip the actual no we specified */
   }
  restbody=chp;mailread=1;
}
			  /* tries to locate the timestamp on the From_ line */
char*findtstamp(start,end)const char*start,*end;
{ end-=25;
  if(*start==' '&&(++start==end||*start==' '&&++start==end))
     return (char*)start-1;
  start=skpspace(start);start+=strcspn(start," \t\n");	/* jump over address */
  if(skpspace(start)>=end)			       /* enough space left? */
     return (char*)start;	 /* no, too small for a timestamp, stop here */
  while(!(end[13]==':'&&end[16]==':')&&--end>start);	  /* search for :..: */
  ;{ int spc=0;						 /* found it perhaps */
     while(end-->start)		      /* now skip over the space to the left */
      { switch(*end)
	 { case ' ':case '\t':spc=1;
	      continue;
	 }
	if(!spc)
	   continue;
	break;
      }
     return (char*)end+1;	   /* this should be right after the address */
   }
}
