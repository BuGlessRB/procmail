/*$Id: goodies.h,v 1.8 1993/06/21 14:24:20 berg Exp $*/

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

extern const char*Tmnate,*All_args;
