/*$Id: robust.h,v 1.5 1993/03/05 14:40:27 berg Exp $*/

void
 *tmalloc Q((const size_t len)),
 *trealloc Q((void*const old,const size_t len)),
 tfree P((void*const p)),
 opnlog P((const char*file));
pid_t
 sfork P((void));
int
 opena P((const char*const a)),
 ropen Q((const char*const name,const mode,const mode_t mask)),
 rpipe P((int fd[2])),
 rdup P((const p)),
 rclose P((const fd)),
 rread P((const fd,void*const a,const len)),
 rwrite P((const fd,const void*const a,const len));
