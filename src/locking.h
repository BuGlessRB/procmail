/*$Id: locking.h,v 1.6 2000/10/24 00:16:43 guenther Exp $*/

void
 lockit P((char*name,char**const lockp)),
 lcllock P((void)),
 unlock P((char**const lockp));
int
 xcreat Q((const char*const name,const mode_t mode,time_t*const tim,
  const chownit));

#ifdef NOfcntl_lock
#ifndef USElockf
#ifndef USEflock
#define fdlock(fd)	0
#define fdunlock()	0
#endif
#endif
#endif
#ifndef fdlock
int
 fdlock P((int fd)),
 fdunlock P((void));
#endif

extern char*globlock;
