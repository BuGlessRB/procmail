/*$Id: sublib.h,v 1.7 1993/01/26 14:49:54 berg Exp $*/

#ifdef NOmemmove
void
 *smemmove Q((void*To,const void*From,size_t count));
#endif

#ifdef NOstrpbrk
char
 *strpbrk P((const char*const st,const char*del));
#endif

#ifdef NOstrstr
char
 *strstr P((const char*whole,const char*const part));
#endif

#ifdef NOstrtol
long
 strtol P((const char*start,const char**const ptr));
#endif
