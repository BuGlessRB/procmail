/*$Id: goodies.h,v 1.17 1999/01/29 22:04:57 guenther Exp $*/

int
 readparse P((char*p,int(*const fpgetc)(),const sarg));
void
 ltstr P((const minwidth,const long val,char*dest)),
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv)),
 retbStdout P((char*const newmyenv)),
 postStdout P((void));
const char
 *sputenv P((const char*const a));
double
 stod P((const char*str,const char**const ptr));

extern long Stdfilled;
extern const char test[];

extern const char*Tmnate,*All_args;
