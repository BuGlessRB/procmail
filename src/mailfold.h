/*$Id: mailfold.h,v 1.4 1992/11/24 16:00:07 berg Exp $*/

long
 dump P((const s,const char*source,long len));
int
 deliver P((char*const boxname)),
 dirmail P((void));
void
 logabstract P((void)),
 concon P((const ch)),
 readmail P((int rhead,const long tobesent));

extern const char scomsat[];
extern logopened,tofile;
extern long lasttell;

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
