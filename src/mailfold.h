/*$Id: mailfold.h,v 1.23 2000/09/28 01:23:28 guenther Exp $*/

long
 dump P((const int s,const int type,const char*source,long len));
int
 writefolder P((char*boxname,char*linkfolder,const char*source,long len,
  const int ignwerr,const int dolock));
void
 logabstract P((const char*const lstfolder)),
 concon P((const int ch)),
 readmail P((int rhead,const long tobesent));
char
 *findtstamp P((const char*start,const char*end));

extern int logopened,rawnonl;
extern off_t lasttell;
