/*$Id: goodies.h,v 1.21 2000/09/28 01:23:23 guenther Exp $*/

int
 readparse P((char*p,int(*const fpgetc)(),const int sarg));
char
 *simplesplit P((char*to,const char*from,const char*fencepost,int*gotp));
void
 ltstr P((const int minwidth,const long val,char*dest)),
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv,int unset)),
 retbStdout P((char*const newmyenv)),
 postStdout P((void));
const char
 *sputenv P((const char*const a)),
 *eputenv P((const char*const src,char*const dst));
double
 stod P((const char*str,const char**const ptr));

extern long Stdfilled;
extern const char test[];

extern const char*Tmnate,*All_args;
