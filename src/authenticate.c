/************************************************************************
 *	Modify to taste in order to comply with your authentication	*
 *	(e.g. Radius or shadow passwd) and mailbox conventions		*
 *									*
 *	You have the liberty to redefine the identity typedef in	*
 *	any way you see fit, so that it can hold state information	*
 *	you need to authenticate your users				*
 *									*
 *	Copyright (c) 1996-1997, S.R. van den Berg, The Netherlands	*
 *	#include "../README" or "README"				*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: authenticate.c,v 1.3 1997/04/03 01:58:39 srb Exp $";
#endif

#ifdef PROCMAIL
#include "includes.h"
#include "robust.h"
#include "shell.h"
#include "misc.h"
#else
#include "config.h"

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#ifdef SHADOW_PASSWD
#include <shadow.h>
#endif
#endif /* PROCMAIL */

#include "authenticate.h"

#ifndef MAILSPOOLDIR
#define MAILSPOOLDIR	"/var/spool/mail/"	     /* watch the trailing / */
#endif
#ifndef MAILSPOOLHASH
#define MAILSPOOLHASH	0      /* 2 would deliver to /var/spool/mail/b/a/bar */
#endif
/*#define MAILSPOOLHOME "/.mail"		      /* watch the leading / */
						  /* delivers to $HOME/.mail */
#define STRLEN(x)	(sizeof(x)-1)

struct auth_identity
{ const struct passwd*pw;
  char*mbox;
  int sock;
};

static auth_identity authi;		      /* reuse copy, only one active */

static void castlower(str)register char*str;
{ for(;*str;str++)
     if((unsigned)*str-'A'<='Z'-'A')
	*str+='a'-'A';
}

static const struct passwd*cgetpwnam(user,sock)const char*const user;
 const int sock;
{ return getpwnam(user);
}

static const struct passwd*cgetpwuid(uid,sock)const uid_t uid;const int sock;
{ return getpwuid(uid);
}

/*const*/auth_identity*auth_finduser(user,sock)char*const user;const int sock;
{ if(!(authi.pw=cgetpwnam(user,sock)))		  /* /etc/passwd user lookup */
   { char*p;
     if(p=strchr(user,'@'))		  /* does the username contain an @? */
	*p='\0';		      /* clueless user using the mailaddress */
     castlower(user);	      /* make it all lowercase (luser problem no. 1) */
     if(!(authi.pw=cgetpwnam(user,sock)))	/* ok, be nice and try again */
	return 0;		       /* sorry, no such user on this planet */
   }
  authi.sock=sock;
  if(authi.mbox)
     free(authi.mbox),authi.mbox=0;
  return &authi;					       /* user found */
}

/*const*/auth_identity*auth_finduid(uid,sock)const uid_t uid;const int sock;
{ if(!(authi.pw=cgetpwuid(uid,sock)))		  /* /etc/passwd user lookup */
     return 0;							     /* nada */
  authi.sock=sock;
  if(authi.mbox)
     free(authi.mbox),authi.mbox=0;
  return &authi;					       /* user found */
}

#ifndef PROCMAIL
int auth_checkpassword(pass,pw,allowemptypw)const auth_identity*const pass;
 const char*const pw;const int allowemptypw;
{ const char*rpw;
  rpw=pass->pw->pw_passwd;
#ifdef SHADOW_PASSWD
  ;{ struct spwd*spwd;
     if(spwd=getspnam(pass->pw->pw_name))
	rpw=spwd->sp_pwdp;
   }
#endif
  if(!*rpw)
     return allowemptypw;
  return !strcmp(rpw,crypt(pw,rpw));
}

const char*auth_getsecret(pass)const auth_identity*const pass;
{ return 0;	       /* no standard way to get a secret, add function here */
}
#else /* PROCMAIL */
auth_identity*auth_newid P((void))
{ auth_identity*pass;
  (pass=malloc(sizeof*pass))->pw=0;pass->mbox=0;
}

void auth_copyid(newpass,oldpass)auth_identity*newpass;
 const auth_identity*oldpass;
{ struct passwd*np;const struct passwd*op;
  if(newpass->mbox)
     free(newpass->mbox),newpass->mbox=0;
  newpass->sock=oldpass->sock;
  if(!(np=(struct passwd*)newpass->pw))
   { np=(struct passwd*)newpass->pw=malloc(sizeof*np);
     np->pw_name=np->pw_dir=np->pw_shell=0;
   }
  np->pw_uid=(op=oldpass->pw)->pw_uid;np->pw_gid=op->pw_gid;
  np->pw_name=cstr(np->pw_name,op->pw_name);
  np->pw_dir=cstr(np->pw_dir,op->pw_dir);
  np->pw_shell=cstr(np->pw_shell,op->pw_shell);
}

void auth_freeid(pass)auth_identity*pass;
{ struct passwd*p;
  if(p=(struct passwd*)pass->pw)
     free(p->pw_name),free(p->pw_dir),free(p->pw_shell),free(p);
  if(pass->mbox)
     free(pass->mbox);
  free(pass);
}

int auth_filledid(pass)const auth_identity*pass;
{ return !!pass->pw;
}
#endif /* PROCMAIL */

const char*auth_mailboxname(pass)auth_identity*const pass;
{ if(!pass->mbox)
#ifdef MAILSPOOLHOME
   { static const char mailfile[]=MAILSPOOLHOME;size_t i;
     if(!(pass->mbox=malloc((i=strlen(pass->pw->pw_dir))+STRLEN(mailfile)+1)))
	return "";
     strcpy(pass->mbox,pass->pw->pw_dir);
     strcpy(pass->mbox+i,mailfile);
#else
   { static const char mailspooldir[]=MAILSPOOLDIR;
     if(!(pass->mbox=malloc(
      STRLEN(mailspooldir)+MAILSPOOLHASH*2+strlen(pass->pw->pw_name)+1)))
	return "";
     strcpy(pass->mbox,mailspooldir);
     ;{ char*p,*n;size_t i;int c;
	for(p=pass->mbox+STRLEN(mailspooldir),n=pass->pw->pw_name,
	 i=MAILSPOOLHASH;i--;*p++='/')
	 { if(*n)
	      c=*n++;
	   *p++=c;
	 }
	strcpy(p,pass->pw->pw_name);
      }
#endif /* MAILSPOOLHOME */
   }
  return pass->mbox;
}

uid_t auth_whatuid(pass)const auth_identity*const pass;
{ return pass->pw->pw_uid;
}

uid_t auth_whatgid(pass)const auth_identity*const pass;
{ return pass->pw->pw_uid;
}

const char*auth_homedir(pass)const auth_identity*const pass;
{ return pass->pw->pw_dir;
}

const char*auth_shell(pass)const auth_identity*const pass;
{ return pass->pw->pw_shell;
}

const char*auth_username(pass)const auth_identity*const pass;
{ return pass->pw->pw_name;
}

void auth_end P((void))
{ if(authi.mbox)
     free(authi.mbox),authi.mbox=0;
  endpwent();
#ifdef SHADOW_PASSWD
  endspwent();
#endif
}