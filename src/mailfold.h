/*$Id: mailfold.h,v 1.22 1999/11/22 19:13:08 guenther Exp $*/

long
 dump P((const int s,const char*source,long len));
int
 rnmbogus P((const char*const name,const struct stat*const stbuf,const int i,
  const int dolog)),
 foldertype P((char*chp,mode_t*const modep,int forcedir,
  struct stat*const paranoid)),
 writefolder P((char*boxname,char*linkfolder,const char*source,long len,
  const int ignwerr,const int dolock));
void
 logabstract P((const char*const lstfolder)),
 concon P((const int ch)),
 readmail P((int rhead,const long tobesent));
char
 *findtstamp P((const char*start,const char*end));

extern int logopened,tofile,rawnonl;
extern off_t lasttell;

#define to_NOTYET	(-3)		     /* spool file doesn't exist yet */
#define to_CANTCREATE	(-2)	/* wrong file type and can't change our mind */
#define to_TOOLONG	(-1)		    /* path + UNIQnamelen > linebuf? */
/*#define to_PIPE	0		    /* program, stdout, or /dev/null */
#define to_MAILDIR	1				   /* maildir folder */
#define to_MH		2					/* MH folder */
#define to_FILE		3					/* real file */
#define to_DIR		4			     /* msg.inode# directory */

#define to_lock(to)	   ((to)>to_MAILDIR)
#define to_atime(to)	   ((to)==to_FILE)	      /* force atime < mtime */
#define to_dotlock(to)	   ((to)==to_FILE)
#define to_delim(to)	   ((to)==to_FILE)
#define to_checkcloser(to) ((to)>to_MH)


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
