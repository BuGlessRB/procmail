/*$Id: sublib.h,v 1.5 1992/11/12 12:29:11 berg Exp $*/

#ifdef NOmemmove
void
 *smemmove Q((void*To,const void*From,size_t count));
#endif

#ifdef NOstrpbrk
char
 *strpbrk P((const char*const st,const char*del));
#endif

#ifdef NOstrtol
long
 strtol P((const char*start,const char**const ptr));
#endif
