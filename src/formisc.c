/************************************************************************
 *	Miscellaneous routines used by formail				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static char rcsid[]="$Id: formisc.c,v 1.6 1992/10/28 17:23:39 berg Exp $";
#endif
#include "includes.h"
#include "formail.h"
#include "sublib.h"
#include "shell.h"
#include "common.h"
#include "ecommon.h"
#include "formisc.h"
						 /* skips an RFC 822 address */
char*skipwords(start,end)const char*start,*const end;
{ int delim='>',firstch;
  if((firstch= *start)=='<')
     goto machref;				 /* machine-usable reference */
  do
   { switch(*start)
      { default:					      /* normal word */
	   if(firstch!='(')	     /* if it did *not* start with a comment */
	    { const char*p;
notend:	      if(p=strpbrk(start,"([\"<,; \t\n"))	/* find next special */
		 switch(*p)			     /* is it a big address? */
		  { case '(':case '[':case '"':start=p;continue;
		    default:return(char*)p;		/* address delimiter */
		  }
	      start=strchr(start,'\0')+1;goto notend; /* it can't be the end */
	    }
	   return(char*)start;	       /* just go passed the leading comment */
	case '(':delim=')';break;				  /* comment */
	case '[':delim=']';break;			   /* domain-literal */
	case '"':delim='"';
      }
machref:
    {int i;
     do
	if((i= *start++)==delim)		 /* corresponding delimiter? */
	   break;
	else if(i=='\\'&&*start)		    /* skip quoted character */
	   ++start;
     while(start<end);						/* anything? */
    }
   }
  while(start<end);
  return(char*)end;
}

void loadsaved(sp)const struct saved*const sp;	     /* load some saved text */
{ switch(*sp->rexp)
   { default:loadchar(' ');	       /* make sure it has leading whitspace */
     case ' ':case '\t':;
   }
  loadbuf(sp->rexp,sp->rexl);
}
							    /* append to buf */
void loadbuf(text,len)const char*const text;const size_t len;
{ if(buffilled+len>buflen)			  /* buf can't hold the text */
     buf=realloc(buf,buflen+=BSIZE);
  tmemmove(buf+buffilled,text,len);buffilled+=len;
}

void loadchar(c)const int c;		      /* append one character to buf */
{ if(buffilled==buflen)
     buf=realloc(buf,buflen+=BSIZE);
  buf[buffilled++]=c;
}

getline P((void))			   /* read a newline-terminated line */
{ if(buflast!=EOF)			     /* do we still have a leftover? */
     loadchar(buflast);				  /* load it into the buffer */
  if(buflast!='\n')
   { int ch;
     while((ch=getchar())!=EOF&&ch!='\n')
	loadchar(ch);				/* load the rest of the line */
     loadchar('\n');		    /* make sure (!), it ends with a newline */
   }		/* (some code in formail.c depends on a terminating newline) */
  return buflast=getchar();			/* look ahead, one character */
}

void elog(a)const char*const a;				     /* error output */
{ fputs(a,stderr);
}

void tputssn(a,l)const char*a;size_t l;
{ while(l--)
     putcs(*a++);
}

void ltputssn(a,l)const char*a;size_t l;
{ if(logsummary)
     totallen+=l;
  else
     putssn(a,l);
}

void lputcs(i)const int i;
{ if(logsummary)
     ++totallen;
  else
     putcs(i);
}

void startprog(argv)const char*const*const argv;
{ int poutfd[2];
  if(!nrtotal)					/* no more mails to display? */
     goto squelch;
  if(nrskip)				  /* should we still skip this mail? */
   { --nrskip;							 /* count it */
squelch:
     opensink();return;
   }
  if(nrtotal>0)
     --nrtotal;							 /* count it */
  dup(oldstdout);pipe(poutfd);
  if(!(child=fork()))	/* DON'T fclose(stdin) here, provokes a bug on HP/UX */
   { close(STDIN);close(oldstdout);close(PWRO);dup(PRDO);close(PRDO);
     shexec(argv);
   }
  close(STDOUT);close(PRDO);
  if(STDOUT!=dup(PWRO)||!(mystdout=fdopen(STDOUT,"a")))
     nofild();
  close(PWRO);
  if(-1==child)
     nlog("Can't fork\n"),exit(EX_OSERR);
}

void nofild P((void))
{ nlog("File table full\n");exit(EX_OSERR);
}

void waitforit P((void))
{ int i;pid_t j;
  while(child!=(j=wait(&i))||WIFSTOPPED(i))
    if(-1==j)
       return;
}

void nlog(a)const char*const a;
{ elog(NAMEPREFIX);elog(a);
}

void logqnl(a)const char*const a;
{ elog(" \"");elog(a);elog("\"\n");
}

void closemine P((void))
{ if((fclose(mystdout)==EOF||errout==EOF)&&!quiet)
     nlog(couldntw),elog("\n"),exit(EX_IOERR);
}

void opensink P((void))
{ if(!(mystdout=fopen(DevNull,"a")))
     nofild();
}
