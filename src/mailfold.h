/*$Id: mailfold.h,v 1.19 1999/06/09 07:44:24 guenther Exp $*/

long
 dump P((const int s,const char*source,long len));
int
 foldertype P((char*chp,mode_t*const modep,int forcedir,int allowlinks)),
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

#define to_CANTCREATE	(-2)	/* wrong file type and can't change our mind */
#define to_TOOLONG	(-1)		    /* path + UNIQnamelen > linebuf? */
/*#define to_PIPE	0				/* program or stdout */
#define to_MAILDIR	1				   /* maildir folder */
#define to_FILE		2					/* real file */
#define to_DIR		3			     /* msg.inode# directory */
#define to_MH		4					/* MH folder */

#define to_lock(to)	   ((to)>to_MAILDIR)
#define to_atime(to)	   ((to)==to_FILE)	      /* force atime < mtime */
#define to_dotlock(to)	   ((to)==to_FILE)
#define to_delim(to)	   ((to)==to_FILE)


#ifdef sMAILBOX_SEPARATOR
#define smboxseparator(fd)	(to_delim(tofile)&&\
 (part=len,rwrite(fd,sMAILBOX_SEPARATOR,STRLEN(sMAILBOX_SEPARATOR))))
#define MAILBOX_SEPARATOR
#else
#define smboxseparator(fd)
#endif /* sMAILBOX_SEPARATOR */
#ifdef eMAILBOX_SEPARATOR
#define emboxseparator(fd)	\
 (to_delim(tofile)&&rwrite(fd,eMAILBOX_SEPARATOR,STRLEN(eMAILBOX_SEPARATOR)))
#ifndef MAILBOX_SEPARATOR
#define MAILBOX_SEPARATOR
#endif
#else
#define emboxseparator(fd)
#endif /* eMAILBOX_SEPARATOR */
