/*$Id: sublib.h,v 1.6 1993/01/22 13:42:55 berg Exp $*/

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
 *pstrstr P((const char*whole,const char*const part));
#endif

#ifdef NOstrtol
long
 strtol P((const char*start,const char**const ptr));
#endif
