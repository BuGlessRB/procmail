/*$Id: goodies.h,v 1.10 1993/11/24 19:46:30 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const sarg)),
 ltstr P((const int minwidth,const long val,char*dest)),
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));

double
 stod P((const char*str,const char**const ptr));

extern long Stdfilled;
#ifndef GOT_bin_test
extern const char test[];
#endif

extern const char*Tmnate,*All_args;
