/*$Id: goodies.h,v 1.13 1994/05/26 14:12:53 berg Exp $*/

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
extern const char test[];

extern const char*Tmnate,*All_args;
