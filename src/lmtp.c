/************************************************************************
 *	LMTP (Local Mail Transfer Protocol) routines			*
 *									*
 *	Copyright (c) 1997,1998,1999 Philip Guenther <guenther@gac.edu> *
 *	This file may be redistributed under the same conditions	*
 *	as the other source files in the procmail suite, with an	*
 *	exception near the bottom of this file which has its own	*
 *	criteria.							*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: lmtp.c,v 1.3 1999/12/17 07:17:14 guenther Exp $"
#endif
#include "procmail.h"
#ifdef LMTP
#include "sublib.h"
#include "robust.h"
#include "misc.h"
#include "shell.h"
#include "authenticate.h"
#include "cstdio.h"
#include "mailfold.h"
#include "memblk.h"
#include "lmtp.h"

/*

lhlo		=> print stuff
mail from:	=> fork
		   Generate From_ line (just ignore size & body type)
rcpt to:	=> deduce pass entry, push it somewhere (static limit,
			returning tempfail beyond?)
data		=> process data till end, if we run out of mem, give 552
			(or 452?) response
bdat		=> allocate requested size buffer & read it in, or 552/452
			reponse.
vrfy		=> just check the user
end of data	=> fork, perform deliveries, giving error codes as you go.
rset		=> if in child, die.


	Should this simply be a _conformant_ implementation or should it
	be _strict_?  For example, after accepting a BDAT, should a DATA
	command cause LMTP to not accept anything but a RSET?  As it stands
	we'll be lenient and simply ignore invalid commands.  It's undefined
	behavior, so we can do what we want.
*/

#define INITIAL_RCPTS	10
#define INCR_RCPTS	20

static int lreaddyn P((long size));

int ctopfd,childserverpid;
char*overread;
size_t overlen;

static int nliseol;		/* is a plain \n the EOL delimiter? */
static char*bufcur;
static const char
 lhlomsg[]=
"250-localhost\r\n\
250-SIZE\r\n\
250-8BITMIME\r\n\
250-PIPELINING\r\n\
250-CHUNKING\r\n\
250 ENHANCEDSTATUSCODES\r\n",
 mustintroducemsg[]="503 5.5.1 You must introduce yourself first\r\n",
 nomemmsg[]="452 4.3.0 insufficient system storage\r\n",
 quitmsg[]="221 2.3.0 Goodbye!\r\n";

static void bufwrite(buffer,len,flush)
const char*buffer;int len;int flush;
{ int already;
  if((already=bufcur-buf2)+len>linebuf||flush)
   { if(already&&already!=rwrite(savstdout,bufcur=buf2,already)||
      len&&len!=rwrite(savstdout,buffer,len))
	exit(EX_OSERR);
   }
  else
   { tmemmove(bufcur,buffer,len);
     bufcur+=len;
   }
}
#define bufinit		bufcur=buf2
#define skiptoeol	do c=getB(); while(c!='\n');  /* skip to end-o'-line */

static int unexpect(str)const char*str;
{ char c;
  while(*str!='\0')
   { if((c=getB())-'A'<='Z'-'A'&&c>='A')c+='a'-'A';
     if(c!=*str)
      { if(c!='\n')
	   skiptoeol;
	return 1;
      }
     str++;
   }
  return 0;
}

static long slurpnumber P((void))
{ int c;long total=0;		/* strtol would require buffering the number */
  while((c=getB())-'0'>=0&&c-'0'<10)
   { total*=10;
     total+=c-'0';
   }
  if(c!=' '&&c!='\n'&&c!='\r')
   { skiptoeol;
     return -1;
   }
  if(c=='\n')
   { ungetb(c);
   }
  return total;
}

/* Slurp "<.*?>" with rfc821 quoting rules, strip the trailing angle bracket */
/* This is stricter than it needs to be, but not as strict as the rfcs */
static char *slurpaddress P((void))
{ char*p=buf,c,*const last=buf+linebuf-1;     /* -1 to leave room for the \0 */
  do{*p=getB();}while(*p==' ');
  if(*p!='<')
   { if(*p!='\n')
	skiptoeol;
     return 0;
   }
  while(++p<last)
     switch(*p=getB())
      { case '\\':
	   if(++p==last)
	      goto syntax_error;
	   *p++=getB();
	   continue;
	case '"':
	   while(++p<last)
	      switch(*p=getB())
	       { case '\\':
		    if(++p==last)
		       goto syntax_error;
		    *p=getB();
		    continue;
		 case '"':
		    break;
		 case '\r':
			goto syntax_error;
		 case '\n':
		    return 0;
		 default:
		    continue;
	       }
	   continue;
	case '\00':case '\01':case '\02':case '\03':case '\04':
	case '\05':case '\06':case '\07':case '\10':case '\11':
		   case '\13':case '\14':case '\15':case '\16':
	case '\17':case '\20':case '\21':case '\22':case '\23':
	case '\24':case '\25':case '\26':case '\27':case '\30':
	case '\31':case '\32':case '\33':case '\34':case '\35':
	case '\36':case '\37':case '<':case '(':case ')':case ';':
syntax_error:
	   skiptoeol;
	case '\n':
	   return 0;
	case '>':
	   *p='\0';				   /* strip the trailing '>' */
	   return tstrdup(buf);
	default:
	   break;
      }
  goto syntax_error;
}

/* given a path from slurpaddress() extract the local-part and strip quoting */
static char *extractaddress(path)char *path;
{ char *p=path+1,*q=path;		       /* +1 to skip the leading '<' */
  if(*p=='@')		    /* route address.  Perhaps we should assume that */
     while(1)			/* sendmail will strip these for recipients? */
	switch(*++p)
	 { case ':':p++;       /* we can take some shortcuts because we know */
	      goto found;		/* the quoting and length to be okay */
	   case '\\':p++;
	      break;
	   case '"':
	      while(*++p!='"')
		 if(*p=='\\')
		    p++;
	      break;
	   case '>':case '\0':
	      free(path);				    /* no local part */
	      return 0;
	 }
found:
  while(1)						    /* strip quoting */
   { switch(*p)
      { case '\\':
	   p++;
	   break;
	case '"':
	   while(*++p!='"')
	      if(*p=='\\')
		 *q++=*++p;
	      else
		 *q++=*p;
	   p++;
	   continue;
	case '\0':case '>':	 /* no final host part?	 That's fine with us */
	case '@':
	   *q='\0';
	   return path;
      }
     *q++=*p++;
   }
}

/*
linebuf MUST be at least 256, and should be at least 1024 or so for buffering
*/

/* LMTP connection states */
#define S_START		0		/* lhlo => S_MAIL */
#define S_MAIL		1		/* mail => S_RCPT */
#define S_RCPT		2		/* data => S_MAIL, bdat => S_BDAT */
#define S_BDAT		3		/* bdat last => S_MAIL */

/* lmtp: run the LTMP protocol.	 It returns in children, never the
   parent.  The return value is the array of recipients, and into the
   first argument is stored a pointer to one past the end of that
   array.  the second argument should be the username of the person
   running procmail (this is used to generate the From_ line)  The
   returning child should handle the deliveries, calling lmtpresponse()
   with the exitcode of each one, then write 'overlen' bytes from
   'overread' to 'ctopfd', and exit.  If something unrecoverable goes
   wrong, and it can't do the necessary calls to lmtpresponse(), then
   it should exit with some non-zero status.  The parent will then
   syslog it, and exit with EX_SOFTWARE.  (See getB() in cstdio.c) */
struct auth_identity **lmtp(lrout,invoker,privs)
struct auth_identity***lrout;char*invoker;int privs;
{ const char*newsendmailflags=0,*msg;
  auth_identity**rcpts,**lastrcpt,**currcpt;
  char*from=0,c;
  int flush=0,lmtp_state=S_START;
  long size=0;

  restartbuf(STDIN);overread=0;overlen=0;nliseol=1;
  bufinit;ctopfd=-1;					 /* setup our output */
  currcpt=rcpts=malloc(INITIAL_RCPTS*sizeof*rcpts);
  lastrcpt=INITIAL_RCPTS+currcpt;
  bufwrite("220 ",4,0);bufwrite(procmailn,strlen(procmailn),0);
  bufwrite(Version,strchr(Version,',')-Version,0);
  bufwrite(" LMTP\r\n",7,1);

  while(1)
   { if(lmtp_state!=S_RCPT&&lmtp_state!=S_BDAT)	    /* clean up our state if */
      { if(from)free(from);			/* we're not doing something */
	newsendmailflags=from=0;size=0;				/* right now */
      }
     do{c=getB();}while(c==' ');
     switch(c)
      { case 'l': case 'L':
	   if(unexpect("hlo "))		 /* we require the space even though */
	      goto unknown_command;  /* we'll just ignore the hostname given */
	   ;{ int sawcrnl=0;		      /* autodetect \r\n vs plain \n */
	      do
	       { c=getB();
		 if(c=='\r')
		  { c=getB();
		    if(c=='\n')
		     { sawcrnl=1;
		       break;
		     }
		  }
	       }
	      while(c!='\n');
	      flush=1;
	      if(lmtp_state!=S_START)
	       { msg="503 5.5.1 LHLO already issued\r\n";
		 goto message;
	       }
	      if(sawcrnl)
		 nliseol=0;
	    }
	   lmtp_state=S_MAIL;
	   msg=lhlomsg;
	   goto message;
	case 'm': case 'M':
	   if(unexpect("ail from:"))
	      goto unknown_command;
	   ;{ int pipefds[2];
	      switch(lmtp_state)
	       { case S_RCPT:case S_BDAT:
		    skiptoeol;
		    msg="503 5.5.1 MAIL sender already specified\r\n";
		    goto message;
		 case S_START:
		    skiptoeol;
		    msg=mustintroducemsg;
		    goto message;
	       }
	      if(!(from=slurpaddress()))
	       { msg="553 5.1.7 Unable to parse MAIL address\r\n";
		 goto message;
	       }
	      goto jumpin;
	      do
	       { switch(c)
		  { case 's':case 'S':
		       if(unexpect("ize="))			  /* rfc1653 */
			  goto unknown_param;
		       size=slurpnumber();
		       if(size<0)		/* will be zerod at loop top */
			  goto unknown_param;
		       break;
		    case 'b':case 'B':
		       if(unexpect("ody="))			  /* rfc1652 */
			  goto unknown_param;
		       while((c=getB())!='\r')		      /* just ignore */
			  switch(c)		      /* the parameter as we */
			   { case ' ':goto jumpin;	/* can't do anything */
			     case '\n':goto jumpout;	   /* useful with it */
			   }
		    case '\r':
		       if((c=getB())=='\n')
			  continue;
		    default:
		       skiptoeol;
unknown_param:	       msg="504 5.5.4 unknown MAIL parameter or bad value\r\n";
		       goto message;
		  }
jumpin:		 do c=getB();
		 while(c==' ');
		}
	      while(c!='\n');
jumpout:      rpipe(pipefds);
	      /*
	       * This is a pipe on which to write back one byte which,
	       * if non-zero, indicates something went wrong and the
	       * parent should act like the MAIL FROM: never happened.
	       * If it was zero then it should be followed by any extra
	       * LMTP commands that the child read past what it needed.
	       */
	      if(!(childserverpid=sfork()))
	       { char status=0;
		 rclose(pipefds[0]);
		 ctopfd=pipefds[1];
		 bufwrite(0,0,1);	  /* free up buf2 for lmtpFrom() */
		 lmtpFrom(from+1,invoker,privs);
		 /* bufinit;	only needed if buf2 might be realloced */
		 if(from)
		    free(from),from=0;
		 if(!size)
		    size=filled;
		 else if(!resizeblock(&themail,size+=filled+3,1))
		 /* try for the memory now */
		  { status=1;		      /* +3 for the "." CRLF */
		    bufwrite(nomemmsg,STRLEN(nomemmsg),1);
		  }
		 if(rwrite(pipefds[1],&status,sizeof(status))!=sizeof(status))
		    exit(EX_OSERR);
		 if(status)
		    exit(0);
		 lmtp_state=S_RCPT;
		 msg="250 2.5.0 MAIL sender ok\r\n";
		 goto message;
	       }
	      rclose(pipefds[1]);
	      if(!forkerr(childserverpid,buf))
	       { char status=1;
		 rread(pipefds[0],&status,sizeof(status));
		 if(!status)
		  { restartbuf(pipefds[0]);	   /* pick up what the child */
		    lmtp_state=S_MAIL;			/* left lying around */
		    bufinit;
		  }
		 continue;				     /* restart loop */
	       }
	      rclose(pipefds[0]);
	      msg="421 4.3.2 unable to fork for MAIL\r\n";
	      goto message;
	    }
	case 'r': case 'R':
	   if((c=getB())=='s'||c=='S')
	    { if(unexpect("et"))
		 goto unknown_command;
	      skiptoeol;
	      if(lmtp_state!=S_START)
		 lmtp_state=S_MAIL;
	      msg="250 2.5.0 RSET OK\r\n";
	      goto message;
	    }
	   if((c!='c'&&c!='C')||unexpect("pt to:"))
	      goto unknown_command;
	   if(lmtp_state!=S_RCPT)
	    { skiptoeol;
	      msg="503 5.5.1 Need MAIL before RCPT\r\n";
	      /* don't change lmtp_state */
	      goto message;
	    }
	   if(currcpt==lastrcpt)		    /* do I need some space? */
	    { int num=lastrcpt-rcpts;
	      rcpts=realloc(rcpts,(num+INCR_RCPTS)*sizeof*rcpts);
	      currcpt=rcpts+num;lastrcpt=currcpt+INCR_RCPTS;
	    }
	   ;{ char *path,*mailbox;auth_identity*temp;
		    /* if it errors, extractaddress() will free its argument */
	      if(!(path=slurpaddress())||!(mailbox=extractaddress(path)))
	       { msg="550 5.1.3 RCPT address syntax error\r\n";
		 goto message;
	       }
/* if we were to handle ESMTP params on the RCPT verb, we would do so here */
	      skiptoeol;
	      if(!(temp=auth_finduser(mailbox,0)))
	       { msg="550 5.1.1 RCPT mailbox unknown\r\n";
		 free(path);
		 goto message;
	       }
	      auth_copyid(*currcpt=auth_newid(),temp);
	      free(path);
	      currcpt++;
	      msg="250 2.1.5 RCPT ok\r\n";
	      goto message;
	    }
	case 'd': case 'D':
	   flush=1;
	   if(unexpect("ata"))
	      goto unknown_command;
	   skiptoeol;
	   switch(lmtp_state)
	    { case S_START:
		 msg=mustintroducemsg;
		 goto message;
	      case S_MAIL:
		 msg="503 5.5.1 DATA from whom?\r\n";
		 goto message;
	      case S_BDAT:
		 msg="503 5.5.1 DATA after BDAT is illegal\r\n";
		 /* rfc1830 says that a RSET is needed at this point */
		 /* we'll just ignore the invalid DATA command */
		 goto message;
	    }
	   if(currcpt==rcpts)
	    { msg="554 5.5.1 DATA to whom?\r\n";
	      goto message;
	    }
	   msg="354 Enter DATA terminated with a solo \".\"\r\n";
	   bufwrite(msg,strlen(msg),1);
	   if(newsendmailflags)
	      flagsendmail=newsendmailflags;
	   if(!(lreaddyn(size)))
	    { /*
	       * At this point we either have more data to read which we
	       * can't fit, or, worse, we've lost part of the command stream.
	       * The (ugly) solution/way out is to send the 452 status code
	       * and then kill both ourselves and out parent.  That's the
	       * only solution short of teaching lreaddyn() to take a small
	       * buffer (buf2?) and repeatedly fill it looking for the end
	       * of the data stream, but that's too ugly.  If the malloc
	       * failed then the machine is probably hurting enough that
	       * our exit can only help.
	       */
	      bufwrite(nomemmsg,STRLEN(nomemmsg),1);
	      goto quit;
	    }
deliver:   readmail(2,0L);		/* fix up things */
	   lastrcpt=rcpts;
	   rcpts=realloc(rcpts,(currcpt-rcpts)*sizeof*rcpts);
	   *lrout=(currcpt-lastrcpt)+rcpts;
	   return rcpts;
	case 'b': case 'B':		/* rfc1830's BDAT */
	   if(unexpect("dat"))
	      goto unknown_command;
	   if((c=getB())!=' ')
	    { if(c!='\n')
		 skiptoeol;
	      msg="504 5.5.4 BDAT octets count missing\r\n";
	      goto message;
	    }
	   switch(lmtp_state)
	    { case S_START:
		 msg=mustintroducemsg;
		 goto message;
	      case S_MAIL:
		 msg="503 5.5.1 BDAT from whom?\r\n";
		 goto message;
	    }
	   if(currcpt==rcpts)
	    { msg="554 5.5.1 BDAT to whom?\r\n";
	      goto message;
	    }
	   ;{ int last=0;
	      long length=slurpnumber();
	      if(length<0)
	       { msg="555 5.5.4 bad value for BDAT octet count\r\n";
		 goto message;
	       }
	      do{c=getB();}while(c==' ');
	      if(c=='l'||c=='L')
	       { if(unexpect("ast"))
		  {
bad_bdat_param:	    msg="504 5.5.4 unknown BDAT parameter\r\n";
		    goto message;
		  }
		 last=1;
		 c=getB();
	       }
	      if(!nliseol&&c=='\r')
		 c=getB();
	      if(c!='\n')
	       { skiptoeol;
		 goto bad_bdat_param;
	       }
	      if(filled+length>size)
	       { if(!resizeblock(&themail,size=filled+length+BLKSIZ,1))
		  { int i;				/* eat the BDAT data */
		    while(length>linebuf)
		     { i=rread(STDIN,buf,linebuf);
		       if(i<0)
			  goto quit;
		       length-=i;
		     }
		    if(length&&0>rread(STDIN,buf,length))
		       goto quit;
		    lmtp_state=S_MAIL;
		    msg=nomemmsg;
		    flush=1;
		    goto message;
		  }
	       }
	      while(length>0)
	       { int i=rread(STDIN,themail.p+filled,length);
		 if(!i)
		    exit(EX_NOINPUT);
		 else if(i<0)
		    goto quit;
		 length-=i;
		 filled+=i;
	       }
	      if(last)
	       { if(!nliseol)				/* change CRNL to NL */
		  { char*in,*out,*q,*last;
		    last=(in=out=themail.p)+filled;
		    while(in<last)
		       if((q=memchr(in,'\r',last-in))?q>in:!!(q=last))
			{ if(in!=out)
			     memmove(out,in,q-in);
			  out+=q-in;in=q;
			}
		       else if(++in==last||*in!='\n')	     /* keep the CR? */
			  *out++='\r';
		    resizeblock(&themail,(filled-=in-out)+1,1);
		  }
		 goto deliver;
	       }
	      msg="250 2.5.0 BDAT chunk okay\r\n";
	      goto message;
	    }
	case 'v': case 'V':
	   if(unexpect("rfy "))
	      goto unknown_command;
	   flush=1;
	   ;{ char *path,*mailbox;
	      auth_identity *temp;
	      if(!(path=slurpaddress())||!(mailbox=extractaddress(path)))
	       { msg="501 5.1.3 VRFY address syntax error\r\n";
		 goto message;
	       }
	      skiptoeol;
	      if(!(temp=auth_finduser(mailbox,0)))
	       { msg="550 5.1.1 VRFY user unknown\r\n";
		 free(path);
		 goto message;
	       }
	      free(path);
	      msg="252 2.5.0 VRFY successful\r\n";
	      goto message;
	    }
	case 'q': case 'Q':
	   if(unexpect("uit"))
	      goto unknown_command;
quit:	   if(ctopfd>=0)	   /* we're the kid: tell the parent to quit */
	    { static const char quitrn[]="quit\r\n";
	      rwrite(ctopfd,quitrn,STRLEN(quitrn));
	      rclose(ctopfd);
	    }
	   else
	      bufwrite(quitmsg,STRLEN(quitmsg),1);
	   exit(0);
	case 'n': case 'N':
	   if(unexpect("oop"))
	      goto unknown_command;
	   skiptoeol;
	   flush=1;
	   msg="200 2.0.0 NOOP?	 Nope\r\n";
	   goto message;
	default:
	   skiptoeol;
unknown_command:
	case '\n':
	   msg="500 5.5.1 Unknown command given\r\n";
	   flush=1;
message:   bufwrite(msg,strlen(msg),flush||endoread());
	   flush=0;
	   continue;
      }
   }
}

#define X(str)	{str,STRLEN(str)}
static struct{const char*mess;int len;}ret2LMTP[]=
{ X("500 5.0.0 usage error\r\n"),				    /* USAGE */
  X("501 5.6.0 data error\r\n"),				  /* DATAERR */
  X("550 5.3.0 input missing\r\n"),				  /* NOINPUT */
  X("550 5.1.1 no such user\r\n"),				   /* NOUSER */
  X("550 5.1.2 no such host\r\n"),				   /* NOHOST */
  X("554 5.0.0 something didn't work\r\n"),		      /* UNAVAILABLE */
  X("554 5.3.0 internal software error\r\n"),			 /* SOFTWARE */
  X("451 4.0.0 OS error\r\n"),					    /* OSERR */
  X("554 5.3.5 system file error\r\n"),				   /* OSFILE */
  X("550 5.0.0 output error\r\n"),				/* CANTCREAT */
  X("451 4.0.0 I/O error\r\n"),					    /* IOERR */
  X("450 4.0.0 deferred\r\n"),					 /* TEMPFAIL */
  X("554 5.5.0 protocol error\r\n"),				 /* PROTOCOL */
  X("550 5.0.0 insufficient permission\r\n"),			   /* NOPERM */
  X("554 5.3.5 configuration error\r\n"),			   /* CONFIG */
};

void lmtpresponse(retcode)int retcode;
{ static const char success[]="250 2.5.0 Message delivered\r\n";
  const char*message;int len;
  if(!retcode)
     message=success,len=STRLEN(success);
  else
   { if(retcode<0)
	retcode=EX_SOFTWARE;
     if(0>(retcode-=EX__BASE)||retcode>=(sizeof ret2LMTP/sizeof ret2LMTP[0]))
	retcode=EX_UNAVAILABLE-EX__BASE;
     message=ret2LMTP[retcode].mess;len=ret2LMTP[retcode].len;
   }
  if(len!=rwrite(savstdout,message,len))
     exit(EX_OSERR);
}

#define IS_NORMAL	0
#define IS_CR		1
#define IS_CRBOL	2
#define IS_CRDOT	3
#define IS_DOTCR	4
#define IS_NLBOL	5
#define IS_NLDOT	6

int lreaddyn(size)long size;
{ int blksiz=BLKSIZ,state,ok,left;unsigned int shift=EXPBLKSIZ;
  state=nliseol?IS_NLBOL:IS_CRBOL;
  if(left=size-filled)			    /* did we have an initial guess? */
   { size=filled;				 /* hopefully avoid resizing */
     goto jumpin;
   }
  for(;;)
   {
#ifdef SMALLHEAP
     if((size_t)size>=(size_t)(size+blksiz))
	lcking|=lck_MEMORY,nomemerr(size);
#endif				       /* dynamically adjust the buffer size */
     while(EXPBLKSIZ&&(ok=0,blksiz>BLKSIZ)&&	   /* backed up all the way? */
      !(ok=resizeblock(&themail,size+blksiz,1)))       /* then try this size */
	blksiz>>=1;			    /* nope, try a smaller increment */
     if(!EXPBLKSIZ||!ok)
	resizeblock(&themail,size+blksiz,0);	    /* last (maybe only) try */
     left=blksiz;
jumpin:
     ;{ int got;
	do
	 { register char*in,*out,*q,*last;
	   if(0>=(got=rread(STDIN,themail.p+size,left)))	/* read mail */
	      return 0;
	   last=(in=out=themail.p+size)+got;
	   /*
	    * A state machine to read SMTP data.  If 'nliseol' is
	    * set then \n is the end-o'-line character and \r is not
	    * special at all.  If 'nliseol' isn't set, then \r\n is the
	    * end-o'-line string, and \n is only special in it.	 \r's are
	    * stripped from \r\n, but are otherwise preserved.
	    */
	   switch(state)
	    { case IS_CR:   goto is_cr;
	      case IS_CRBOL:goto is_crbol;
	      case IS_CRDOT:goto is_crdot;
	      case IS_DOTCR:goto is_dotcr;
	      case IS_NLBOL:goto is_nlbol;
	      case IS_NLDOT:goto is_nldot;
	      case IS_NORMAL:break;
	    }
#define EXIT_LOOP(s)	{state=(s);goto loop_exit;}
	   if(!nliseol)
	      while(in<last)
		 if((q=memchr(in,'\r',last-in))?q>in:!!(q=last))
		  { if(in!=out)
		       memmove(out,in,q-in);
		    out+=q-in;in=q;
		  }
		 else						       /* CR */
		  {
found_cr:	    *out++=*in++;		   /* tenatively save the \r */
		    if(in==last)
		       EXIT_LOOP(IS_CR)
is_cr:		    if(*in!='\n')
		       continue;
		    out[-1]=*in++;			 /* overwrite the \r */
		    if(in==last)				     /* CRLF */
		       EXIT_LOOP(IS_CRBOL)
is_crbol:	    if(*in=='\r')				 /* CRLF CR? */
		       goto found_cr;
		    if(*in!='.')
		     { *out++=*in++;
		       continue;
		     }
		    if(++in==last)				 /* CRLF "." */
		       EXIT_LOOP(IS_CRDOT)
is_crdot:	    if((*out++=*in++)!='\r')
		       continue;
		    if(in==last)			      /* CRLF "." CR */
		       EXIT_LOOP(IS_DOTCR)
is_dotcr:	    if(*in=='\n')			    /* CRLF "." CRLF */
		     { out--;			   /* remove the trailing \r */
		       goto found_end;
		     }
		  }
	   else /* nliseol */
	      while(in<last)
		 if((q=memchr(in,'\r',last-in))?q>in:!!(q=last))
		  { if(in!=out)
		       memmove(out,in,q-in);
		    out+=q-in;in=q;
		  }
		 else						       /* LF */
		  { do
		     { *out++=*in++;
is_nlbol:	       ;
		     }
		    while(in<last&&*in=='\n');
		    if(in==last)
		       EXIT_LOOP(IS_NLBOL)
		    if(*in!='.')
		     { *out++=*in++;
		       continue;
		     }
		    if(++in==last)				   /* LF "." */
		       EXIT_LOOP(IS_NLDOT)
is_nldot:	    if(*in=='\n')				/* LF "." LF */
found_end:	     { size=out-themail.p;
		       if((overlen=last-++in)>0) /* should never be negative */
			  tmemmove(overread=malloc(overlen),in,overlen);
		       goto eoffound;
		     }
		    *out++=*in++;
		  }
	   state=IS_NORMAL;	 /* we must have fallen out because in==last */
loop_exit:
	   got-=in-out;			     /* correct for what disappeared */
	 }
	while(size+=got,left-=got);		/* change listed buffer size */
      }
     if(EXPBLKSIZ&&shift)				 /* room for growth? */
      { int newbs=blksiz;newbs<<=shift--;	/* capped exponential growth */
	if(blksiz<newbs)				  /* no overflowing? */
	   blksiz=newbs;				    /* yes, take me! */
      }
   }
eoffound:
  resizeblock(&themail,(filled=size)+1,1);    /* minimise+1 for housekeeping */
  return 1;
}
#else
int lmtp_dummy_var;		      /* to prevent insanity in some linkers */
#endif
