/************************************************************************
 *	Routines that deal with the mailfolder(format)			*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: mailfold.c,v 1.55 1994/08/23 12:16:12 berg Exp $";
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
#include "mailfold.h"
#ifndef NO_COMSAT
#include "network.h"
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
	dif=escFrom_.offs[i]-dist;		     /* the whole next block */
	break;
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
	len--,part--,rwrite(s,source++,1),ssleep(1);	    /* a_time<m_time */
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
     if(tofile==to_FOLDER&&len&&lasttell>=0&&!ftruncate(s,lasttell))
	nlog("Truncated file to former size\n");    /* undo appended garbage */
     i=rclose(s);
   }			   /* return an error even if nothing was to be sent */
  tofile=0;
  return i&&!len?-1:len;
}

static int dirfile(chp,linkonly)char*const chp;const int linkonly;
{ const static char lkingto[]="Linking to";
  if(chp)
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
	while((ok=linkonly?link(buf2,buf):hlink(buf2,buf))&&errno==EEXIST);
	if(linkonly)
	 { yell(lkingto,buf);
	   if(ok)
	      goto nolnk;
	   goto ret;
	 }
      }
     unlink(buf2);
     goto opn;
   }
  ;{ struct stat stbuf;
     stat(buf2,&stbuf);
     ultoan((unsigned long)stbuf.st_ino,      /* filename with i-node number */
      strchr(strcat(buf,msgprefix),'\0'));
   }
  if(linkonly)
   { yell(lkingto,buf);
     if(link(buf2,buf))	   /* hardlink the new file, it's a directory folder */
nolnk:	nlog("Couldn't make link to"),logqnl(buf);
     goto ret;
   }
  if(!rename(buf2,buf))		       /* rename it, we need the same i-node */
opn: return opena(buf);
ret:
  return -1;
}

static int ismhdir(chp)char*const chp;
{ if(chp-1>=buf&&chp[-1]==*MCDIRSEP_&&*chp==chCURDIR)
   { chp[-1]='\0';
     return 1;
   }
  return 0;
}
				       /* open file or new file in directory */
int deliver(boxname,linkfolder)char*boxname,*linkfolder;
{ struct stat stbuf;char*chp;int mhdir;mode_t numask;
  asgnlastf=1;
  if(*boxname=='|'&&(!linkfolder||linkfolder==Tmnate))
   { setlastfolder(boxname);
     return rdup(savstdout);
   }
  numask=UPDATE_MASK&~cumask;tofile=to_FILE;
  if(boxname!=buf)
     strcpy(buf,boxname);		 /* boxname can be found back in buf */
  if(*(chp=buf))				  /* not null length target? */
     chp=strchr(buf,'\0')-1;		     /* point to just before the end */
  mhdir=ismhdir(chp);				      /* is it an MH folder? */
  if(!stat(boxname,&stbuf))					/* it exists */
   { if(numask&&!(stbuf.st_mode&UPDATE_MASK))
	chmod(boxname,stbuf.st_mode|UPDATE_MASK);
     if(!S_ISDIR(stbuf.st_mode))	 /* it exists and is not a directory */
	goto makefile;				/* no, create a regular file */
   }
  else if(!mhdir||mkdir(buf,NORMdirperm))    /* shouldn't it be a directory? */
makefile:
   { if(linkfolder)	  /* any leftovers?  Now is the time to display them */
	concatenate(linkfolder),skipped(linkfolder);
     tofile=strcmp(devnull,buf)?to_FOLDER:0;
     return opena(boxname);
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
     if(unique(buf2,strchr(buf2,'\0'),NORMperm,verbose,0)&&
      (fd=dirfile(chp,0))>=0&&linkfolder)	 /* save the file descriptor */
	for(strcpy(buf2,buf),boxname=linkfolder;boxname!=Tmnate;)
	 { strcpy(buf,boxname);		/* go through the list of other dirs */
	   if(*(chp=buf))
	      chp=strchr(buf,'\0')-1;
	   mhdir=ismhdir(chp);			      /* is it an MH folder? */
	   if(stat(boxname,&stbuf))			 /* it doesn't exist */
	      mkdir(buf,NORMdirperm);				/* create it */
	   else if(numask&&!(stbuf.st_mode&UPDATE_MASK))
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
#ifndef NO_COMSAT
  ;{ int s;struct sockaddr_in addr;char*chp,*chad;	     /* @ seperator? */
     if(chad=strchr(chp=(char*)scomsat,SERV_ADDRsep))
	*chad++='\0';		      /* split it up in service and hostname */
     else if(!renvint(-1L,chp))			/* or is it a false boolean? */
	return;					       /* ok, no comsat then */
     else
	chp="";				  /* set to yes, so take the default */
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
	 { endhostent();		     /* host can't be found, too bad */
	   return;
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
	 { endservent();
	   return;
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

static int concnd;				 /* last concatenation value */

void concon(ch)const int ch;   /* flip between concatenated and split fields */
{ size_t i;
  if(concnd!=ch)				   /* is this run redundant? */
   { concnd=ch;			      /* no, but note this one for next time */
     for(i=confield.filled;i;)		   /* step through the saved offsets */
	themail[confield.offs[--i]]=ch;		       /* and flip every one */
   }
}

static void ffrom(chp)const char*chp;
{ while(chp=strstr(chp,FROM_EXPR))
     app_val(&escFrom_,(off_t)(++chp-themail));	       /* bogus From_ found! */
}

void readmail(rhead,tobesent)const long tobesent;
{ char*chp,*pastend,*realstart;static size_t contlengthoffset;
  ;{ long dfilled;
     if(rhead)					/* only read in a new header */
      { dfilled=mailread=0;chp=readdyn(malloc(1),&dfilled);filled-=tobesent;
	if(tobesent<dfilled)		   /* adjust buffer size (grow only) */
	 { realstart=themail;
	   thebody=(themail=realloc(themail,dfilled+filled))+
	    (thebody-realstart);
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
	    { case '\t':case ' ':app_val(&confield,(off_t)(thebody-1-themail));
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
  escFrom_.filled=0;
  ;{ int i;				    /* eradicate From_ in the header */
     i= *thebody;*thebody='\0';ffrom(realstart);*thebody=i;
   }
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
  ffrom(chp);mailread=1;	  /* eradicate From_ in the rest of the body */
}
			  /* tries to locate the timestamp on the From_ line */
char*findtstamp(start,end)const char*start,*end;
{ start=skpspace(start);start+=strcspn(start," \t\n");	/* jump over address */
  if(skpspace(start)>=(end-=25))		       /* enough space left? */
     return (char*)start;	 /* no, too small for a timestamp, stop here */
  while(!(end[13]==':'&&end[15]==':')&&--end>start);	  /* search for :..: */
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
