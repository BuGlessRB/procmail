/*$Id: goodies.h,v 1.22 2000/10/23 09:04:19 guenther Exp $*/

int
 readparse P((char*p,int(*const fpgetc)(),const int sarg));
char
 *simplesplit P((char*to,const char*from,const char*fencepost,int*gotp));
void
 ltstr P((const int minwidth,const long val,char*dest));
double
 stod P((const char*str,const char**const ptr));

extern const char test[];

extern const char*Tmnate,*All_args;
