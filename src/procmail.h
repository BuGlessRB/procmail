/*$Id: procmail.h,v 1.19 1993/11/24 19:46:55 berg Exp $*/

#include "includes.h"

typedef unsigned char uschar;	     /* sometimes uchar is already typedef'd */
#ifdef uchar
#undef uchar
#endif
#define uchar uschar

#ifdef console
#define DEFverbose 1
#else
#define DEFverbose 0
#endif

#ifndef DEFsendmail
#define DEFsendmail SENDMAIL
#endif

#ifndef SYSTEM_MBOX
#define SYSTEM_MBOX	SYSTEM_MAILBOX
#endif

#ifndef ETCRC
#define ETCRC	0
#endif

#define MAX32	((long)(~(unsigned long)0>>1))
#define MIN32	(-(long)MAX32)

#define XTRAlinebuf	2	     /* surplus of LINEBUF (see readparse()) */

#define rc_NOSGID	1		      /* you can forget any sgidness */
#define rc_NORMAL	2

#define MCDIRSEP	(dirsep+STRLEN(dirsep)-1)      /* most common DIRSEP */
#define MCDIRSEP_	(dirsep+STRLEN(DIRSEP)-1)

#define lck_LOCKFILE	1	  /* crosscheck the order of this with msg[] */
#define lck_ALLOCLIB	2		      /* in sterminate() in retint.c */
#define lck_MEMORY	4
#define lck_FORK	8
#define lck_FILDES	16
#define lck_KERNEL	32

extern struct varval{const char*const name;long val;}strenvvar[];
#define locksleep	(strenvvar[0].val)
#define locktimeout	(strenvvar[1].val)
#define suspendv	(strenvvar[2].val)
#define noresretry	(strenvvar[3].val)
#define timeoutv	(strenvvar[4].val)
#define verbose		(*(volatile long*)&strenvvar[5].val)
#define lgabstract	(strenvvar[6].val)

struct dyna_long{size_t filled,tspace;off_t*offs;};

int
 eqFrom_ P((const char*const a));

extern char*buf,*buf2,*loclock,*tolock,*Stdout,*themail,*thebody;
extern const char shellflags[],shell[],lockfile[],lockext[],newline[],binsh[],
 unexpeof[],shellmetas[],*const*gargv,*const*restargv,*sgetcp,*rcfile,
 dirsep[],msgprefix[],devnull[],lgname[],executing[],oquote[],cquote[],
 whilstwfor[],procmailn[],Mail[],home[],maildir[],*defdeflock,*argv0;
extern long filled,lastscore;
extern sh,pwait,retval,retvl2,lcking,rcstate,rc,ignwerr,lexitcode,asgnlastf,
 accspooldir,crestarg,skiprc,savstdout;
extern size_t linebuf;
extern volatile nextexit;
extern pid_t thepid;
extern uid_t uid;
extern gid_t gid,sgid;

/*
 *	External variables that are checked/changed by the signal handlers:
 *	volatile time_t alrmtime;
 *	pid_t pidfilt,pidchild;
 *	volatile nextexit;
 *	int lcking;
 *	static volatile mailread;	in mailfold.c
 */
