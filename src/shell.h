/*$Id: shell.h,v 1.2 1992/10/02 14:41:14 berg Exp $*/

#define malloc(n)	tmalloc((size_t)(n))
#define realloc(p,n)	trealloc(p,(size_t)(n))
#define free(p)		tfree(p)
#define tmemmove(t,f,n) memmove(t,f,(size_t)(n))
