/************************************************************************
 *	Environment and variable handling routines used by procmail	*
 *									*
 *	Copyright (c) 1990-1999, S.R. van den Berg, The Netherlands	*
 *	Copyright (c) 2000, Philip Guenther, The United States		*
 *							of America	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: variables.c,v 1.1 2000/10/23 09:04:26 guenther Exp $";
#endif
#include "procmail.h"
#include "acommon.h"		/* for hostname() */
#include "cstdio.h"
#include "robust.h"
#include "shell.h"
#include "authenticate.h"
#include "goodies.h"
#include "misc.h"
#include "pipes.h"		/* for exitcode */
#include "variables.h"

long Stdfilled;
static struct dynstring*myenv;
static char**lastenv;
			      /* smart putenv, the way it was supposed to be */
const char*sputenv(a)const char*const a;
{ static int alloced;size_t eq,i;int remove;const char*split;char**preenv;
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
  return empty;
}
	   /* between calling primeStdout() and retStdout() *no* environment */
void primeStdout(varname)const char*const varname;   /* changes are allowed! */
{ if(!Stdout)
     sputenv(varname);
  Stdout=(char*)myenv;
  Stdfilled=ioffsetof(struct dynstring,ename[0])+strlen(varname);
}

void retStdout(newmyenv,unset)			/* see note on primeStdout() */
 char*const newmyenv;int unset;
{ if(unset)					     /* on second thought... */
   { myenv=((struct dynstring*)newmyenv)->enext;	 /* pull it back out */
     free(newmyenv);*lastenv=Stdout=0;
     return;
   }
  if(newmyenv[Stdfilled-1]=='\n')	       /* strip one trailing newline */
     Stdfilled--;
  retbStdout(newmyenv);
}

void retbStdout(newmyenv)char*const newmyenv;	/* see note on primeStdout() */
{ newmyenv[Stdfilled]='\0';*lastenv=(myenv=(struct dynstring*)newmyenv)->ename;
  Stdout=0;
}

void postStdout P((void))		 /* throw it into the keyword parser */
{ const char*p;size_t i;
  p= *lastenv;tmemmove(buf,p,i=strchr(p,'=')-p);buf[i]='\0';asenv(p+i+1);
}

const char*eputenv(src,dst)const char*const src;char*const dst;
{ sgetcp=src;
  return readparse(dst,sgetc,2)?0:sputenv(buf);
}

void cleanupenv(preserve)int preserve;
{ static const char*const keepenv[]=KEEPENV,*const ld_[]=LDENV;
  const char**emax=(const char**)environ,**ep,*const*pp;
  register const char*p;
  size_t i;
  if(!preserve)					     /* drop the environment */
   { for(pp=keepenv;*pp;pp++)			     /* preserve a happy few */
      { i=strlen(*pp);
	for(ep=emax;p= *ep;ep++)		     /* scan for this keeper */
	   if(!strncmp(*pp,p,i)&&(p[i]=='='||p[i-1]=='_'))
	    { *ep= *emax;					 /* swap 'em */
	      *emax++=p;
	    }					   /* keep scanning for dups */
      }
     *emax=0;						    /* drop the rest */
   }
  else				/* keep the environment, but still drop LD_* */
   { ep=emax;
     while(*emax)			  /* find the end of the environment */
	emax++;
     while(*ep)
      { for(pp=ld_;p= *pp;pp++)
	   if(!strncmp(*ep,p,strlen(p))) /* if it starts with LD_ or similar */
	    { *ep= *--emax;*emax=0;			/* copy from the end */
	      goto recheck;			  /* check the swapped entry */
	    }
	ep++;
recheck:;
      }
   }
}

void setupenv(pass,fallback,do_presets)auth_identity*pass;
 const char*fallback;int do_presets;
{ static const char*const prestenv[]=PRESTENV;
  const char*p,*const*pp;
  if(pass)
   { p=auth_username(pass);
     if(!p||!*p)
	p=fallback;
     setdef(lgname,p);
     p=auth_shell(pass);
     if(!p||!*p)
	p=binsh;
     setdef(shell,p);
     setdef(home,auth_homedir(pass));setdef(orgmail,auth_mailboxname(pass));
   }
  else
   { setdef(lgname,fallback);setdef(shell,binsh);
     setdef(home,ROOT_DIR);setdef(orgmail,DEAD_LETTER);
   }
  if(do_presets)
   { setdef(host,hostname());
     sputenv(lastfolder);
     sputenv(exitcode);
     initdefenv();
     for(pp=prestenv;*pp;pp++)		/* preset some environment variables */
	if(!eputenv(*pp,buf))
	   setoverflow();
   }
}
