/*$Id: ecommon.h,v 1.1 1992/10/28 17:23:30 berg Exp $*/

void
 *tmalloc Q((const size_t len)),
 *trealloc Q((void*old,const size_t len)),
 tfree P((void*a));
int
 mystrstr P((const char*whole,const char*const part,const char*end));
