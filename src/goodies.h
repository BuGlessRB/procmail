/*$Id: goodies.h,v 1.5 1992/11/19 12:33:03 berg Exp $*/

void
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));
int
 readparse P((char*p,int(*const fpgetc)(),const int sarg)),
 waitfor Q((const pid_t pid));

extern long Stdfilled;
#ifndef GOT_bin_test
extern const char test[];
#endif

#define TMNATE		'\377'
