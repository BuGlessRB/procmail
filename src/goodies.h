/*$Id: goodies.h,v 1.9 1993/08/20 11:22:51 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const sarg)),
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));

extern long Stdfilled;
#ifndef GOT_bin_test
extern const char test[];
#endif

extern const char*Tmnate,*All_args;
