/*$Id: goodies.h,v 1.7 1993/05/07 13:18:04 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const sarg)),
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));
int
 waitfor Q((const pid_t pid));

extern long Stdfilled;
#ifndef GOT_bin_test
extern const char test[];
#endif

#define TMNATE		'\377'
