/*$Id: mailfold.h,v 1.2 2002/06/30 06:47:04 guenther Exp $*/

long
 dump P((const int s,const int type,const char*source,long len));
int
 writefolder P((char*boxname,char*linkfolder,const char*source,long len,
  const int ignwerr,const int dolock)),
 readmail P((int rhead,const long tobesent));
void
 logabstract P((const char*const lstfolder)),
 concon P((const int ch));

extern int logopened,rawnonl;
extern off_t lasttell;
