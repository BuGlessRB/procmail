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
 "$Id: authenticate.c,v 1.1 1997/01/01 14:52:51 srb Exp $";
#endif

#ifdef PROCMAIL
#include "includes.h"
#include "shell.h"
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

#define STRLEN(x)	(sizeof(x)-1)

static struct auth_identity
{ const struct passwd*pw;
  char*mbox;
  int sock;
} auth_identity authi;			      /* reuse copy, only one active */


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

const auth_identity*auth_finduser(user,sock)char*const user;const int sock;
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

const auth_identity*auth_finduid(uid,sock)const uid_t uid;const int sock;
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
#endif /* PROCMAIL */

static const char mailspooldir[]=MAILSPOOLDIR;

const char*auth_mailboxname(pass)auth_identity*const pass;
{ if(!pass->mbox)
   { if(!(pass->mbox=malloc(STRLEN(mailspooldir)+strlen(pass->pw->pw_name))))
	return "";
     strcpy(pass->mbox,mailspooldir);
     strcpy(pass->mbox+STRLEN(mailspooldir),pass->pw->pw_name);
   }
  return pass->mbox;
}

const char*const auth_mailboxinfo P((void))   /* informational purposes only */
{ return mailspooldir;	     /* returned text is to be displayed to the user */
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
