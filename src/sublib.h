/*$Id: sublib.h,v 1.8 1994/05/26 13:48:31 berg Exp $*/

#ifdef NOmemmove
void
 *smemmove Q((void*To,const void*From,size_t count));
#endif

#ifdef NOstrpbrk
char
 *strpbrk P((const char*const st,const char*del));
#endif

#ifdef SLOWstrstr
char
 *sstrstr P((const char*const phaystack,const char*const pneedle));
#endif

#ifdef NOstrtol
long
 strtol P((const char*start,const char**const ptr));
#endif
