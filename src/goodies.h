/*$Id: goodies.h,v 1.19 1999/11/08 07:06:08 guenther Exp $*/

int
 readparse P((char*p,int(*const fpgetc)(),const int sarg));
void
 ltstr P((const int minwidth,const long val,char*dest)),
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv,int unset)),
 retbStdout P((char*const newmyenv)),
 postStdout P((void));
const char
 *sputenv P((const char*const a));
double
 stod P((const char*str,const char**const ptr));

extern long Stdfilled;
extern const char test[];

extern const char*Tmnate,*All_args;
