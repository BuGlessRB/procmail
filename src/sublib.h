/*$Id: sublib.h,v 1.2 1992/10/02 14:41:20 berg Exp $*/

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
