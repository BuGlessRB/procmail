/************************************************************************
 *	Collection of library-worthy routines				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: goodies.c,v 1.15 1993/06/21 14:24:17 berg Exp $";
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
#ifndef GOT_bin_test
const char test[]="test";
#endif
const char*Tmnate,*All_args;

#define NOTHING_YET	(-1)	 /* readparse understands a very complete    */
#define SKIPPING_SPACE	0	 /* subset of the standard /bin/sh syntax    */
#define NORMAL_TEXT	1	 /* that includes single-, double- and back- */
#define DOUBLE_QUOTED	2	 /* quotes, backslashes and $subtitutions    */
#define SINGLE_QUOTED	3

#define fgetc() (*fpgetc)()	   /* some compilers previously choked on it */

/* sarg==0 : normal parsing, split up arguments like in /bin/sh
 * sarg==1 : environment assignment parsing, parse up till first whitespace
 * sarg==2 : normal parsing, split up arguments by single spaces
 */
void readparse(p,fpgetc,sarg)register char*p;int(*const fpgetc)();
 const int sarg;
{ static i;int got;char*startb;
  All_args=0;
  for(got=NOTHING_YET;;)		    /* buf2 is used as scratch space */
loop:
   { i=fgetc();
     if(buf+linebuf-3<p)	    /* doesn't catch everything, just a hint */
      { nlog("Exceeded LINEBUF\n");p=buf+linebuf-3;goto ready;
      }
newchar:
     switch(i)
      { case EOF:	/* check sarg too to prevent warnings in the recipe- */
	   if(sarg!=2&&got>NORMAL_TEXT)		 /* condition expansion code */
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
	    }
	   if(got>NORMAL_TEXT)
noesc:	      *p++='\\';		/* nothing to escape, just echo both */
	   break;
	case '`':
	   if(got==SINGLE_QUOTED)
	      goto nodelim;
	   for(startb=p;;)			       /* mark your position */
	    { switch(i=fgetc())			 /* copy till next backquote */
	       { case '\\':
		    switch(i=fgetc())
		     { case EOF:nlog(unexpeof);goto forcebquote;
		       case '\n':continue;
		       case '"':
			  if(got!=DOUBLE_QUOTED)
			     break;
		       case '\\':case '$':case '`':goto escaped;
		     }
		    *p++='\\';break;
		 case '"':
		    if(got!=DOUBLE_QUOTED)     /* missing closing backquote? */
		       break;
forcebquote:	 case EOF:case '`':
		  { int osh=sh;
		    *p='\0';
		    if(!(sh=!!strpbrk(startb,tgetenv(shellmetas))))
		     { const char*save=sgetcp,*sall_args=All_args;
		       sgetcp=p=tstrdup(startb);readparse(startb,sgetc,0);
		       if(!All_args)	       /* only one can be remembered */
			  All_args=sall_args;		    /* this is a bug */
#ifndef GOT_bin_test
		       if(!strcmp(test,startb))
			  strcpy(startb,p),sh=1;       /* oops, `test' found */
#endif
		       free(p);sgetcp=save;		       /* chopped up */
		     }		    /* drop source buffer, read from program */
		    startb=
		     fromprog(p=startb,startb,(size_t)(buf-startb+linebuf-3));
		    sh=osh;				       /* restore sh */
		    if(got!=DOUBLE_QUOTED)
		     { i=0;startb=p;goto simplsplit;	      /* split it up */
		     }
		    if(i=='"'||got<=SKIPPING_SPACE)   /* missing closing ` ? */
		       got=NORMAL_TEXT;			     /* or sarg!=0 ? */
		    p=startb;goto loop;
		  }
		 case '\n':i=';';	       /* newlines separate commands */
	       }
escaped:      *p++=i;
	    }
	case '"':
	   switch(got)
	    { case DOUBLE_QUOTED:got=NORMAL_TEXT;continue;	/* closing " */
	      case SINGLE_QUOTED:goto nodelim;
	    }
	   got=DOUBLE_QUOTED;continue;				/* opening " */
	case '\'':
	   switch(got)
	    { case DOUBLE_QUOTED:goto nodelim;
	      case SINGLE_QUOTED:got=NORMAL_TEXT;continue;}	/* closing ' */
	   got=SINGLE_QUOTED;continue;				/* opening ' */
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
		 All_args=p;continue;
	      case '{':						  /* ${name} */
		 while(EOF!=(i=fgetc())&&alphanum(i))
		    *startb++=i;
		 *startb='\0';
		 if(i!='}'||numeric(*buf2)&&buf2[1])
		  { nlog("Bad substitution of");logqnl(buf2);continue;
		  }
		 i='\0';break;					  /* $$ =pid */
	      case '$':ultstr(0,(unsigned long)thepid,p);goto ieofstr;
	      case '?':strcpy(p,"-1");
		 if(lexitcode>=0)  /* $? =exitcode from last started program */
		    ultstr(0,(unsigned long)lexitcode,p);
		 goto ieofstr; /* $# =number of extra command-line arguments */
	      case '#':ultstr(0,(unsigned long)crestarg,p);goto ieofstr;
	      case '-':strcpy(p,tgetenv(lastfolder));
ieofstr:	 i='\0';goto eofstr;			   /* $- =lastfolder */
	      default:
		 if(numeric(i))			   /* $n positional argument */
		  { *startb++=i;i='\0';goto finsb;
		  }
		 if(alphanum(i))				    /* $name */
		  { do *startb++=i;
		    while(EOF!=(i=fgetc())&&alphanum(i));
		    if(i==EOF)
			i='\0';
finsb:		    *startb='\0';break;
		  }
normchar:	 *p++='$';goto newchar;		       /* not a substitution */
	    }
	   ;{ int j;
	      startb=(unsigned)(j=(*buf2)-'0')>9?(char*)tgetenv(buf2):
	       !j?(char*)argv0:j<=crestarg?(char*)restargv[j-1]:"";
	    }
	   if(got!=DOUBLE_QUOTED)
simplsplit:   for(;;startb++)		  /* simply split it up in arguments */
	       { switch(*startb)
		  { case ' ':case '\t':case '\n':
		       if(got<=SKIPPING_SPACE)
			  continue;
		       *p++=sarg?' ':'\0';got=SKIPPING_SPACE;continue;
		    case '\0':goto eeofstr;
		  }
		 *p++= *startb;got=NORMAL_TEXT;
	       }
	   else
	    { strcpy(p,startb);				   /* simply copy it */
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

waitfor(pid)const pid_t pid;		      /* wait for a specific process */
{ int i;pid_t j;
  while(pid!=(j=wait(&i))||WIFSTOPPED(i))
     if(-1==j)
	return -1;
  return lexitcode=WIFEXITED(i)?WEXITSTATUS(i):-1;
}

static struct lienv{struct lienv*enext;char ename[255];}*myenv;
static char**lastenv;
			      /* smart putenv, the way it was supposed to be */
void sputenv(a)const char*const a;
{ static alloced;int i,remove;char*split,**preenv;struct lienv*curr,**last;
  yell("Assigning",a);remove=0;
  if(!(split=strchr(a,'=')))			   /* assignment or removal? */
     remove=1,split=strchr(a,'\0');
  i=split-a;
  for(curr= *(last= &myenv);curr;curr= *(last= &curr->enext))	/* is it one */
     if(!strncmp(a,curr->ename,i)&&curr->ename[i]=='=')	  /* I made earlier? */
      { split=curr->ename;*last=curr->enext;free(curr);
	for(preenv=environ;*preenv!=split;preenv++);
	goto wipenv;
      }
  for(preenv=environ;*preenv;preenv++)		    /* is it in the standard */
     if(!strncmp(a,*preenv,i)&&(*preenv)[i]=='=')	     /* environment? */
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
     curr=malloc(ioffsetof(struct lienv,ename[0])+(i=strlen(a)+1));
     tmemmove(*(lastenv=preenv)=curr->ename,a,i);preenv[1]=0;curr->enext=myenv;
     myenv=curr;
   }
}
			    /* between calling primeStdout() and retStdout() */
void primeStdout P((void))	    /* *no* environment changes are allowed! */
{ char*p;
  if((p=strchr(buf,'\0'))[-1]!='=')		   /* does it end in an '='? */
     *p='=',p[1]='\0';					/* make sure it does */
  sputenv(buf);Stdout=(char*)myenv;
  Stdfilled=ioffsetof(struct lienv,ename[0])+strlen(myenv->ename);
}

void retStdout(newmyenv)char*const newmyenv;	/* see note on primeStdout() */
{ if(newmyenv[Stdfilled-1]=='\n')	       /* strip one trailing newline */
     Stdfilled--;
  newmyenv[Stdfilled]='\0';*lastenv=(myenv=(struct lienv*)newmyenv)->ename;
  Stdout=0;
}

void postStdout P((void))		 /* throw it into the keyword parser */
{ const char*p;size_t i;
  p= *lastenv;tmemmove(buf,p,i=strchr(p,'=')-p);buf[i]='\0';asenv(p+i+1);
}
