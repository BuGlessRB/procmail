/************************************************************************
 *	Routines that deal with the mailfolder(format)			*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: mailfold.c,v 1.32 1993/07/01 11:58:32 berg Exp $";
#endif
#include "procmail.h"
#include "sublib.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "exopen.h"
#include "goodies.h"
#include "locking.h"
#include "mailfold.h"
#ifndef NO_COMSAT
#include "network.h"

const char scomsat[]="COMSAT";
#endif
int logopened,tofile;
off_t lasttell;
static long lastdump;
static volatile mailread;	/* if the mail is completely read in already */
static struct dyna_long escFrom_,confield;	  /* escapes, concatenations */
			       /* inserts escape characters on outgoing mail */
static long getchunk(s,fromw,len)const int s;const char*fromw;const long len;
{ long dist,dif;int i;static const char esc[]=ESCAP;
  dist=fromw-themail;			/* where are we now in transmitting? */
  for(dif=len,i=0;i<escFrom_.filled;)	    /* let's see if we can find this */
     if(!(dif=escFrom_.offs[i++]-dist))			 /* this exact spot? */
      { rwrite(s,esc,STRLEN(esc));lastdump++;			/* escape it */
	if(i>=escFrom_.filled)				      /* last block? */
	   return len;				/* yes, give all what's left */
	dif=escFrom_.offs[i]-dist;break;	     /* the whole next block */
      }
     else if(dif>0)				/* passed this spot already? */
	break;
  return dif<len?dif:len;
}

long dump(s,source,len)const int s;const char*source;long len;
{ int i;long part;
  lasttell=i= -1;
  if(s>=0)
   { if(tofile&&(lseek(s,(off_t)0,SEEK_END),fdlock(s)))
	nlog("Kernel-lock failed\n");
     lastdump=len;part=tofile==to_FOLDER?getchunk(s,source,len):len;
     lasttell=lseek(s,(off_t)0,SEEK_END);smboxseparator(s);	 /* optional */
#ifndef NO_NFS_ATIME_HACK					/* separator */
     if(part&&tofile)		       /* if it is a file, trick NFS into an */
	len--,part--,rwrite(s,source++,1),sleep(1);	    /* a_time<m_time */
#endif
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
     if(!len&&(lastdump<2||!(source[-1]=='\n'&&source[-2]=='\n')))
	lastdump++,rwrite(s,newline,1);	       /* message always ends with a */
     emboxseparator(s);		 /* newline and an optional custom separator */
writefin:
     if(tofile&&fdunlock())
	nlog("Kernel-unlock failed\n");
     i=rclose(s);
   }			   /* return an error even if nothing was to be sent */
  tofile=0;return i&&!len?-1:len;
}

static dirfile(chp,linkonly)char*const chp;const int linkonly;
{ if(chp)
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
     ;{ int ok;
	do ultstr(0,++i,chp);		       /* find first empty MH folder */
	while((ok=link(buf2,buf))&&errno==EEXIST);
	if(linkonly)
	 { if(ok)
	      goto nolnk;
	   goto ret;
	 }
      }
     unlink(buf2);goto opn;
   }
  ;{ struct stat stbuf;
     stat(buf2,&stbuf);
     ultoan((unsigned long)stbuf.st_ino,      /* filename with i-node number */
      strchr(strcat(buf,tgetenv(msgprefix)),'\0'));
   }
  if(linkonly)
   { yell("Linking to",buf);
     if(link(buf2,buf))	   /* hardlink the new file, it's a directory folder */
nolnk:	nlog("Couldn't make link to"),logqnl(buf);
     goto ret;
   }
  if(!myrename(buf2,buf))	       /* rename it, we need the same i-node */
opn: return opena(buf);
ret:
  return -1;
}

static ismhdir(chp)char*const chp;
{ if(chp-1>=buf&&chp[-1]==*MCDIRSEP_&&*chp==chCURDIR)
   { chp[-1]='\0';return 1;
   }
  return 0;
}
				       /* open file or new file in directory */
deliver(boxname,linkfolder)char*boxname,*linkfolder;
{ struct stat stbuf;char*chp;int mhdir;mode_t cumask;
  umask(cumask=umask(0));cumask=UPDATE_MASK&~cumask;tofile=to_FILE;
  asgnlastf=1;
  if(boxname!=buf)
     strcpy(buf,boxname);		 /* boxname can be found back in buf */
  if(*(chp=buf))				  /* not null length target? */
     chp=strchr(buf,'\0')-1;		     /* point to just before the end */
  mhdir=ismhdir(chp);				      /* is it an MH folder? */
  if(!stat(boxname,&stbuf))					/* it exists */
   { if(cumask&&!(stbuf.st_mode&UPDATE_MASK))
	chmod(boxname,stbuf.st_mode|UPDATE_MASK);
     if(!S_ISDIR(stbuf.st_mode))	 /* it exists and is not a directory */
	goto makefile;				/* no, create a regular file */
   }
  else if(!mhdir||mkdir(buf,NORMdirperm))    /* shouldn't it be a directory? */
makefile:
   { if(linkfolder)	  /* any leftovers?  Now is the time to display them */
	concatenate(linkfolder),skipped(linkfolder);
     tofile=strcmp(devnull,buf)?to_FOLDER:0;return opena(boxname);
   }
  if(linkfolder)		    /* any additional directories specified? */
   { size_t blen;
     if(blen=Tmnate-linkfolder)		       /* copy the names into safety */
	Tmnate=(linkfolder=tmemmove(malloc(blen),linkfolder,blen))+blen;
     else
	linkfolder=0;
   }
  if(mhdir)				/* buf should contain directory name */
     *chp='\0',chp[-1]= *MCDIRSEP_,strcpy(buf2,buf);	   /* it ended in /. */
  else					 /* fixup directory name, append a / */
     strcat(chp,MCDIRSEP_),strcpy(buf2,buf),chp=0;
  ;{ int fd= -1;		/* generate the name for the first directory */
     if(unique(buf2,strchr(buf2,'\0'),NORMperm,verbose)&&
      (fd=dirfile(chp,0))>=0&&linkfolder)	 /* save the file descriptor */
	for(strcpy(buf2,buf),boxname=linkfolder;boxname!=Tmnate;)
	 { strcpy(buf,boxname);		/* go through the list of other dirs */
	   if(*(chp=buf))
	      chp=strchr(buf,'\0')-1;
	   mhdir=ismhdir(chp);			      /* is it an MH folder? */
	   if(stat(boxname,&stbuf))			 /* it doesn't exist */
	      mkdir(buf,NORMdirperm);				/* create it */
	   else if(cumask&&!(stbuf.st_mode&UPDATE_MASK))
	      chmod(buf,stbuf.st_mode|UPDATE_MASK);
	   if(mhdir)
	      *chp='\0',chp[-1]= *MCDIRSEP_;
	   else				 /* fixup directory name, append a / */
	      strcat(chp,MCDIRSEP_),chp=0;
	   dirfile(chp,1);		/* link it with the original in buf2 */
	   while(*boxname++);		  /* skip to the next directory name */
	 }
     if(linkfolder)					   /* free our cache */
	free(linkfolder);
     return fd;			      /* return the file descriptor we saved */
   }
}

void logabstract(lstfolder)const char*const lstfolder;
{ if(lgabstract>0||logopened&&lgabstract)  /* don't mail it back unrequested */
   { char*chp,*chp2;int i;static const char sfolder[]=FOLDER;
     if(mailread)			  /* is the mail completely read in? */
      { *thebody='\0';		       /* terminate the header, just in case */
	if(eqFrom_(chp=themail))		       /* any "From " header */
	 { if(chp=strchr(themail,'\n'))
	      *chp++='\0';
	   else
	      chp=thebody;			  /* preserve mailbox format */
	   elog(themail);elog(newline);			     /* (any length) */
	 }
	if(!(lcking&lck_ALLOCLIB)&&		/* don't reenter malloc/free */
	 (chp=egrepin(NSUBJECT,chp,(long)(thebody-chp),0)))
	 { for(chp2= --chp;*--chp2!='\n'&&*chp2;);
	   if(chp-++chp2>MAXSUBJECTSHOW)	    /* keep it within bounds */
	      chp2[MAXSUBJECTSHOW]='\0';
	   *chp='\0';detab(chp2);elog(" ");elog(chp2);elog(newline);
	 }
      }
     elog(sfolder);
     i=strlen(strncpy(buf,lstfolder,MAXfoldlen))+STRLEN(sfolder);
     buf[MAXfoldlen]='\0';detab(buf);elog(buf);i-=i%TABWIDTH;	/* last dump */
     do elog(TABCHAR);
     while((i+=TABWIDTH)<LENoffset);
     ultstr(7,lastdump,buf);elog(buf);elog(newline);
   }
#ifndef NO_COMSAT
  ;{ int s;struct sockaddr_in addr;char*chp,*chad;	     /* @ seperator? */
     if(chad=strchr(chp=(char*)tgetenv(scomsat),SERV_ADDRsep))
	*chad++='\0';		      /* split it up in service and hostname */
     else if(!renvint(-1L,scomsat))		/* or is it a false boolean? */
	return;					       /* ok, no comsat then */
     if(!chad||!*chad)						  /* no host */
#ifndef IP_localhost
	chad=COMSAThost;				      /* use default */
#else /* IP_localhost */
      { static const unsigned char ip_localhost[]=IP_localhost;
	addr.sin_family=AF_INET;
	tmemmove(&addr.sin_addr,ip_localhost,sizeof ip_localhost);
      }
     else
#endif /* IP_localhost */
      { const struct hostent*host;	      /* what host?  paranoid checks */
	if(!(host=gethostbyname(chad))||!host->h_0addr_list)
	 { endhostent();return;		     /* host can't be found, too bad */
	 }
	addr.sin_family=host->h_addrtype;	     /* address number found */
	tmemmove(&addr.sin_addr,host->h_0addr_list,host->h_length);
	endhostent();
      }
     if(!*chp)						       /* no service */
	chp=BIFF_serviceport;				      /* use default */
     s=strtol(chp,&chad,10);
     if(chp==chad)			       /* the service is not numeric */
      { const struct servent*serv;
	if(!(serv=getservbyname(chp,COMSATprotocol)))	   /* so get its no. */
	 { endservent();return;
	 }
	addr.sin_port=serv->s_port;endservent();
      }
     else
	addr.sin_port=htons((short)s);			    /* network order */
     cat(tgetenv(lgname),"@");			 /* should always fit in buf */
     if(lasttell>=0)					   /* was it a file? */
	ultstr(0,(unsigned long)lasttell,buf2),catlim(buf2);	      /* yep */
     catlim(COMSATxtrsep);				 /* custom seperator */
     if(lasttell>=0&&!strchr(dirsep,*lstfolder))       /* relative filename? */
	catlim(tgetenv(maildir)),catlim(MCDIRSEP_);   /* prepend current dir */
     catlim(lstfolder);s=socket(AF_INET,SOCK_DGRAM,UDP_protocolno);
     sendto(s,buf,strlen(buf),0,(const void*)&addr,sizeof(addr));rclose(s);
     yell("Notified comsat:",buf);
   }
#endif /* NO_COMSAT */
}

static concnd;					 /* last concatenation value */

void concon(ch)const int ch;   /* flip between concatenated and split fields */
{ size_t i;
  if(concnd!=ch)				   /* is this run redundant? */
   { concnd=ch;			      /* no, but note this one for next time */
     for(i=confield.filled;i;)		   /* step through the saved offsets */
	themail[confield.offs[--i]]=ch;		       /* and flip every one */
   }
}

void readmail(rhead,tobesent)const long tobesent;
{ char*chp,*pastend,*realstart;
  ;{ long dfilled;
     if(rhead)					/* only read in a new header */
      { dfilled=mailread=0;chp=readdyn(malloc(1),&dfilled);filled-=tobesent;
	if(tobesent<dfilled)		   /* adjust buffer size (grow only) */
	   themail=realloc(themail,dfilled+filled);
	tmemmove(themail+dfilled,thebody,filled);tmemmove(themail,chp,dfilled);
	free(chp);themail=realloc(themail,1+(filled+=dfilled));
      }
     else
      { if(!mailread||!filled)
	   rhead=1;	 /* yup, we read in a new header as well as new mail */
	mailread=0;dfilled=thebody-themail;themail=readdyn(themail,&filled);
      }
     pastend=filled+(thebody=themail);
     while(thebody<pastend&&*thebody++=='\n');	     /* skip leading garbage */
     realstart=thebody;
     if(rhead)			      /* did we read in a new header anyway? */
      { confield.filled=0;concnd='\n';
	while(thebody=
	 egrepin("[^\n]\n[\n\t ]",thebody,(long)(pastend-thebody),1))
	   if(thebody[-1]!='\n')		  /* mark continuated fields */
	      app_val(&confield,(off_t)(--thebody-1-themail));
	   else
	      goto eofheader;		   /* empty line marks end of header */
	thebody=pastend;      /* provide a default, in case there is no body */
eofheader:;
      }
     else			       /* no new header read, keep it simple */
	thebody=themail+dfilled; /* that means we know where the body starts */
   }
  ;{ int f1stchar;    /* to make sure that the first From_ line is uninjured */
     f1stchar= *realstart;*(chp=realstart)='\0';escFrom_.filled=0;
     while(chp=egrepin(FROM_EXPR,chp,(long)(pastend-chp),1))
      { while(*--chp!='\n');		       /* where did this line start? */
	app_val(&escFrom_,(off_t)(++chp-themail));chp++;	   /* bogus! */
      }
     *realstart=f1stchar;mailread=1;
   }
}
