/************************************************************************
 *	Collection of library-worthy routines				*
 *									*
 *	Copyright (c) 1990-1994, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: goodies.c,v 1.32 1994/05/26 14:12:51 berg Exp $";
#endif
#include "procmail.h"
#include "sublib.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#include "pipes.h"
#include "common.h"
#include "cstdio.h"
#include "goodies.h"

long Stdfilled;
const char test[]="test";
const char*Tmnate,*All_args;

static const char*evalenv P((void))	/* expects the variable name in buf2 */
{ int j;
  return (unsigned)(j=(*buf2)-'0')>9?getenv(buf2):
	  !j?argv0:
	   j<=crestarg?restargv[j-1]:(const char*)0;
}

#define NOTHING_YET	(-1)	 /* readparse understands a very complete    */
#define SKIPPING_SPACE	0	 /* subset of the standard /bin/sh syntax    */
#define NORMAL_TEXT	1	 /* that includes single-, double- and back- */
#define DOUBLE_QUOTED	2	 /* quotes, backslashes and $subtitutions    */
#define SINGLE_QUOTED	3

#define fgetc() (*fpgetc)()	   /* some compilers previously choked on it */

/* sarg==0 : normal parsing, split up arguments like in /bin/sh
 * sarg==1 : environment assignment parsing, parse up till first whitespace
 * sarg==2 : normal parsing, split up arguments by existing whitespace
 */
void readparse(p,fpgetc,sarg)register char*p;int(*const fpgetc)();
 const int sarg;
{ static i,skipbracelev;int got,bracelev,qbracelev;char*startb;
  static char*skipback;static const char*oldstartb;
  bracelev=qbracelev=0;All_args=0;
  for(got=NOTHING_YET;;)		    /* buf2 is used as scratch space */
loop:
   { i=fgetc();
     if(buf+linebuf-3<p)	    /* doesn't catch everything, just a hint */
      { nlog("Exceeded LINEBUF\n");p=buf+linebuf-3;goto ready;
      }
newchar:
     switch(i)
      { case EOF:	/* check sarg too to prevent warnings in the recipe- */
	   if(sarg<2&&got>NORMAL_TEXT)		 /* condition expansion code */
early_eof:    nlog(unexpeof);
ready:	   if(got!=SKIPPING_SPACE||sarg)  /* not terminated yet or sarg==2 ? */
	      *p++='\0';
	   Tmnate=p;return;
	case '\\':
	   if(got==SINGLE_QUOTED)
	      break;
	   switch(i=fgetc())
	    { case EOF:goto early_eof;			  /* can't quote EOF */
	      case '\n':continue;			/* concatenate lines */
	      case '#':
		 if(got>SKIPPING_SPACE) /* escaped comment at start of word? */
		    goto noesc;			/* apparently not, literally */
	      case ' ':case '\t':case '\'':
		 if(got==DOUBLE_QUOTED)
		    goto noesc;
	      case '"':case '\\':case '$':case '`':goto nodelim;
	      case '}':
		 if(got<=NORMAL_TEXT&&bracelev||
		    got==DOUBLE_QUOTED&&bracelev>qbracelev)
		    goto nodelim;
	    }
	   if(got>NORMAL_TEXT)
noesc:	      *p++='\\';		/* nothing to escape, just echo both */
	   break;
	case '`':
	   if(got==SINGLE_QUOTED)
	      goto nodelim;
	   for(startb=p;;)			       /* mark your position */
	    { switch(i=fgetc())			 /* copy till next backquote */
	       { case '"':
		    if(got!=DOUBLE_QUOTED)     /* missing closing backquote? */
		       break;
forcebquote:	 case EOF:case '`':
		    if(skiprc)
		       *(p=startb)='\0';
		    else
		     { int osh=sh;
		       *p='\0';
		       if(!(sh=!!strpbrk(startb,shellmetas)))
			{ const char*save=sgetcp,*sAll_args;
			  sgetcp=p=tstrdup(startb);sAll_args=All_args;
			  readparse(startb,sgetc,0);All_args=sAll_args;
#ifndef GOT_bin_test
			  if(!strcmp(test,startb))
			     strcpy(startb,p),sh=1;    /* oops, `test' found */
#endif
			  free(p);sgetcp=save;		       /* chopped up */
			}	    /* drop source buffer, read from program */
		       startb=fromprog(
			p=startb,startb,(size_t)(buf-startb+linebuf-3));
		       sh=osh;				       /* restore sh */
		     }
		    if(got!=DOUBLE_QUOTED)
		     { i=0;startb=p;goto simplsplit;	      /* split it up */
		     }
		    if(i=='"'||got<=SKIPPING_SPACE)   /* missing closing ` ? */
		       got=NORMAL_TEXT;
		    p=startb;goto loop;
		 case '\\':
		    switch(i=fgetc())
		     { case EOF:nlog(unexpeof);goto forcebquote;
		       case '\n':continue;
		       case '"':
			  if(got!=DOUBLE_QUOTED)
			     break;
		       case '\\':case '$':case '`':goto escaped;
		     }
		    *p++='\\';
	       }
escaped:      *p++=i;
	    }
	case '"':
	   switch(got)
	    { case DOUBLE_QUOTED:
		 if(qbracelev<bracelev)		   /* still inside a ${...}? */
	      case SINGLE_QUOTED:
		    goto nodelim;				 /* nonsense */
		 got=NORMAL_TEXT;continue;			/* closing " */
	    }
	   qbracelev=bracelev;got=DOUBLE_QUOTED;continue;	/* opening " */
	case '\'':
	   switch(got)
	    { case DOUBLE_QUOTED:goto nodelim;
	      case SINGLE_QUOTED:got=NORMAL_TEXT;continue;	/* closing ' */
	    }
	   got=SINGLE_QUOTED;continue;				/* opening ' */
	case '}':
	   if(got<=NORMAL_TEXT&&bracelev||
	      got==DOUBLE_QUOTED&&bracelev>qbracelev)
	    { bracelev--;
	      if(skipback&&bracelev==skipbracelev)
	       { skiprc--;p=skipback;skipback=0;startb=(char*)oldstartb;
		 goto closebrace;
	       }
	      continue;
	    }
	   goto nodelim;
	case '#':
	   if(got>SKIPPING_SPACE)		/* comment at start of word? */
	      break;
	   while((i=fgetc())!=EOF&&i!='\n');		    /* skip till EOL */
	   goto ready;
	case '$':
	   if(got==SINGLE_QUOTED)
	      break;
	   startb=buf2;
	   switch(i=fgetc())
	    { case EOF:*p++='$';goto ready;
	      case '@':
		 if(got!=DOUBLE_QUOTED)
		    goto normchar;
		 if(!skiprc)	      /* don't do it while skipping (braces) */
		    All_args=p;
		 continue;
	      case '{':						  /* ${name} */
		 while(EOF!=(i=fgetc())&&alphanum(i))
		    *startb++=i;
		 *startb='\0';startb=(char*)evalenv();
		 if(numeric(*buf2)&&buf2[1])
		    goto badsub;
		 switch(i)
		  { default:goto badsub;
		    case ':':
		       switch(i=fgetc())
			{ default:
badsub:			     nlog("Bad substitution of");logqnl(buf2);continue;
			  case '-':
			     if(startb&&*startb)
				goto noalt;
			     goto doalt;
			  case '+':
			     if(startb&&*startb)
				goto doalt;
			     startb=0;
			}
		    case '+':
		       if(startb)
			  goto doalt;
		       goto noalt;
		    case '-':
		       if(startb)
noalt:			  if(!skiprc)
			   { skiprc++;skipback=p;skipbracelev=bracelev;
			     oldstartb=startb;
			   }
doalt:		       bracelev++;continue;
		    case '}':
closebrace:	       if(!startb)
			  startb="";
		  }
		 goto ibreak;					  /* $$ =pid */
	      case '$':ultstr(0,(unsigned long)thepid,p);goto ieofstr;
	      case '?':ltstr(0,(long)lexitcode,p);goto ieofstr;
	      case '#':ultstr(0,(unsigned long)crestarg,p);goto ieofstr;
	      case '=':ltstr(0,lastscore,p);
ieofstr:	 i='\0';goto eofstr;
	      case '_':startb=incnamed?incnamed->ename:"";goto ibreak;
	      case '-':startb=(char*)tgetenv(lastfolder); /* $- =$LASTFOLDER */
ibreak:		 i='\0';break;
	      default:
		 if(numeric(i))			   /* $n positional argument */
		  { *startb++=i;i='\0';goto finsb;
		  }
		 if(alphanum(i))				    /* $name */
		  { do *startb++=i;
		    while(EOF!=(i=fgetc())&&alphanum(i));
		    if(i==EOF)
			i='\0';
finsb:		    *startb='\0';
		    if(!(startb=(char*)evalenv()))
		       startb="";
		    break;
		  }
normchar:	 *p++='$';goto newchar;		       /* not a substitution */
	    }
	   if(got!=DOUBLE_QUOTED)
simplsplit: { if(sarg)
		 goto copyit;
	      for(;;startb++)		  /* simply split it up in arguments */
	       { switch(*startb)
		  { case ' ':case '\t':case '\n':
		       if(got<=SKIPPING_SPACE)
			  continue;
		       *p++='\0';got=SKIPPING_SPACE;continue;
		    case '\0':goto eeofstr;
		  }
		 *p++= *startb;got=NORMAL_TEXT;
	       }
	    }
	   else
copyit:	    { strcpy(p,startb);				   /* simply copy it */
eofstr:	      if(got<=SKIPPING_SPACE)		/* can only occur if sarg!=0 */
		 got=NORMAL_TEXT;
	      p=strchr(p,'\0');
	    }
eeofstr:   if(i)			     /* already read next character? */
	      goto newchar;
	   continue;
	case ' ':case '\t':
	   switch(got)
	    { case NORMAL_TEXT:
		 if(sarg==1)
		    goto ready;		/* already fetched a single argument */
		 got=SKIPPING_SPACE;*p++=sarg?' ':'\0';	 /* space or \0 sep. */
	      case NOTHING_YET:case SKIPPING_SPACE:continue;   /* skip space */
	    }
	case '\n':
	   if(got<=NORMAL_TEXT)
	      goto ready;			    /* EOL means we're ready */
      }
nodelim:
     *p++=i;					   /* ah, a normal character */
     if(got<=SKIPPING_SPACE)		 /* should we bother to change mode? */
	got=NORMAL_TEXT;
   }
}

void ltstr(minwidth,val,dest)const int minwidth;const long val;char*dest;
{ if(val<0)
   { *dest=' ';ultstr(minwidth-1,-val,dest+1);
     while(*++dest==' ');		     /* look for the first non-space */
     dest[-1]='-';				  /* replace it with a minus */
   }
  else
     ultstr(minwidth,val,dest);				/* business as usual */
}

double stod(str,ptr)const char*str;const char**const ptr;
{ int sign,any;unsigned i;char*chp;double acc,fracc;
  fracc=1;acc=any=sign=0;
  switch(*(chp=skpspace(str)))					 /* the sign */
   { case '-':sign=1;
     case '+':chp++;
   }
  while((i=(unsigned)*chp++-'0')<=9)		 /* before the decimal point */
     acc=acc*10+i,any=1;
  switch(i)
   { case (unsigned)'.'-'0':case (unsigned)','-'0':
	while(fracc/=10,(i=(unsigned)*chp++-'0')<=9)  /* the fractional part */
	   acc+=fracc*i,any=1;
   }
  if(ptr)
     *ptr=any?chp-1:str;
  return sign?-acc:acc;
}

static struct dynstring*myenv;
static char**lastenv;
			      /* smart putenv, the way it was supposed to be */
const char*sputenv(a)const char*const a;
{ static alloced;size_t eq,i;int remove;const char*split;char**preenv;
  struct dynstring*curr,**last;
  yell("Assigning",a);remove=0;
  if(!(split=strchr(a,'=')))			   /* assignment or removal? */
     remove=1,split=strchr(a,'\0');
  eq=split-a;							    /* is it */
  for(curr= *(last= &myenv);curr;curr= *(last= &curr->enext))  /* one I made */
     if(!strncmp(a,curr->ename,eq)&&((char*)curr->ename)[eq]=='=')
      { split=curr->ename;*last=curr->enext;free(curr);		 /* earlier? */
	for(preenv=environ;*preenv!=split;preenv++);
	goto wipenv;
      }
  for(preenv=environ;*preenv;preenv++)		    /* is it in the standard */
     if(!strncmp(a,*preenv,eq)&&(*preenv)[eq]=='=')	     /* environment? */
wipenv:
      { while(*preenv=preenv[1])   /* wipe this entry out of the environment */
	   preenv++;
	break;
      }
  i=(preenv-environ+2)*sizeof*environ;
  if(alloced)		   /* have we ever alloced the environ array before? */
     environ=realloc(environ,i);
  else
     alloced=1,environ=tmemmove(malloc(i),environ,i-sizeof*environ);
  if(!remove)		  /* if not remove, then add it to both environments */
   { for(preenv=environ;*preenv;preenv++);
     preenv[1]=0;*(lastenv=preenv)=(char*)(split=newdynstring(&myenv,a));
     return split+eq+1;
   }
  return "";
}
			    /* between calling primeStdout() and retStdout() */
void primeStdout P((void))	    /* *no* environment changes are allowed! */
{ char*p;
  if((p=strchr(buf,'\0'))[-1]!='=')		   /* does it end in an '='? */
     *p='=',p[1]='\0';					/* make sure it does */
  sputenv(buf);Stdout=(char*)myenv;
  Stdfilled=ioffsetof(struct dynstring,ename[0])+strlen(myenv->ename);
}

void retStdout(newmyenv)char*const newmyenv;	/* see note on primeStdout() */
{ if(newmyenv[Stdfilled-1]=='\n')	       /* strip one trailing newline */
     Stdfilled--;
  newmyenv[Stdfilled]='\0';*lastenv=(myenv=(struct dynstring*)newmyenv)->ename;
  Stdout=0;
}

void postStdout P((void))		 /* throw it into the keyword parser */
{ const char*p;size_t i;
  p= *lastenv;tmemmove(buf,p,i=strchr(p,'=')-p);buf[i]='\0';asenv(p+i+1);
}
