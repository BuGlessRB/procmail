/*$Id: goodies.h,v 1.11 1994/04/05 15:34:40 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const sarg)),
 ltstr P((const int minwidth,const long val,char*dest)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));
const char
 *sputenv P((const char*const a));
double
 stod P((const char*str,const char**const ptr));

extern long Stdfilled;
#ifndef GOT_bin_test
extern const char test[];
#endif

extern const char*Tmnate,*All_args;
