/*$Id: mailfold.h,v 1.13 1994/10/14 18:43:35 berg Exp $*/

long
 dump P((const s,const char*source,long len));
int
 deliver P((char*boxname,char*linkfolder));
void
 logabstract P((const char*const lstfolder)),
 concon P((const ch)),
 readmail P((int rhead,const long tobesent));
char
 *findtstamp P((const char*start,const char*end));

extern int logopened,tofile,rawnonl,dumperr;
extern off_t lasttell;

#define to_FILE		1		  /* when we are writing a real file */
#define to_FOLDER	2		 /* when we are writing a filefolder */

#ifdef sMAILBOX_SEPARATOR
#define smboxseparator(fd)	(tofile==to_FOLDER&&\
 (part=len,rwrite(fd,sMAILBOX_SEPARATOR,STRLEN(sMAILBOX_SEPARATOR))))
#define MAILBOX_SEPARATOR
#else
#define smboxseparator(fd)
#endif /* sMAILBOX_SEPARATOR */
#ifdef eMAILBOX_SEPARATOR
#define emboxseparator(fd)	\
 (tofile==to_FOLDER&&rwrite(fd,eMAILBOX_SEPARATOR,STRLEN(eMAILBOX_SEPARATOR)))
#ifndef MAILBOX_SEPARATOR
#define MAILBOX_SEPARATOR
#endif
#else
#define emboxseparator(fd)
#endif /* eMAILBOX_SEPARATOR */
