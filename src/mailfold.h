/*$Id: mailfold.h,v 1.17 1999/04/19 06:42:18 guenther Exp $*/

long
 dump P((const int s,const char*source,long len));
int
 foldertype P((char*const chp)),
 writefolder P((char*boxname,char*linkfolder,const char*source,const long len,
  const int ignwerr));
void
 logabstract P((const char*const lstfolder)),
 concon P((const int ch)),
 readmail P((int rhead,const long tobesent));
char
 *findtstamp P((const char*start,const char*end));

extern int logopened,tofile,rawnonl;
extern off_t lasttell;

#define to__LOCK	(1<<2)			   /* fdlock before writing? */
#define to__DIR		(1<<3)			     /* must exist or mkdir? */
#define to__ATIME	(1<<4)			      /* force atime < mtime */
#define to__OLDSTYLEDIR (to__LOCK|to__ATIME)

#define to_TOOLONG	(-1)		    /* path + UNIQnamelen > linebuf? */
/*#define to_PIPE	0				/* program or stdout */
#define to_FILE		(to__LOCK)				/* real file */
#define to_MAILDIR	(to__DIR)			   /* maildir folder */
#define to_DIR		(to__OLDSTYLEDIR)	     /* msg.inode# directory */
#define to_MH		(to__OLDSTYLEDIR|to__DIR)		/* MH folder */

#define to_overflow(to)	   ((to)<0)
#define to_shouldbedir(to) ((to)&to__DIR)
#define to_lock(to)	   ((to)&to__LOCK)
#define to_doatime(to)	   ((to)&to__ATIME)
#define to_dotlock(to)	   ((to)==to_FILE)
#define to_dodelim(to)	   ((to)==to_FILE)


#ifdef sMAILBOX_SEPARATOR
#define smboxseparator(fd)	(to_dodelim(tofile)&&\
 (part=len,rwrite(fd,sMAILBOX_SEPARATOR,STRLEN(sMAILBOX_SEPARATOR))))
#define MAILBOX_SEPARATOR
#else
#define smboxseparator(fd)
#endif /* sMAILBOX_SEPARATOR */
#ifdef eMAILBOX_SEPARATOR
#define emboxseparator(fd)	\
 (to_dodelim(tofile)&&rwrite(fd,eMAILBOX_SEPARATOR,STRLEN(eMAILBOX_SEPARATOR)))
#ifndef MAILBOX_SEPARATOR
#define MAILBOX_SEPARATOR
#endif
#else
#define emboxseparator(fd)
#endif /* eMAILBOX_SEPARATOR */
