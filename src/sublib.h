2/*$Id: sublib.h,v 1.4 1992/11/12 12:28:04 berg Exp $*/

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
