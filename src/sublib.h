/*$Id: sublib.h,v 1.1 1992/09/28 14:28:00 berg Exp $*/

#ifdef NOmemmove
void
 *smemmove Q((void*To,void*From,size_t count));
#endif

#ifdef NOstrpbrk
char
 *strpbrk P((const char*const st,const char*del));
#endif

#ifdef NOstrtol
long
 strtol P((const char*start,const char**const ptr));
#endif
