/*$Id: goodies.h,v 1.16 1996/12/27 02:53:24 srb Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const sarg)),
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
