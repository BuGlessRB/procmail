/*$Id: ecommon.h,v 1.2 1992/11/11 13:59:13 berg Exp $*/

void
 *tmalloc Q((const size_t len)),
 *trealloc Q((void*old,const size_t len)),
 tfree P((void*a));
int
 mystrstr P((const char*whole,const char*const part,const char*end));
