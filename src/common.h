/*$Id: common.h,v 1.7 1994/01/11 13:17:00 berg Exp $*/

void
 shexec P((const char*const*argv)),
 detab P((char*p)),
 ultstr P((int minwidth,unsigned long val,char*dest));
char
 *skpspace P((const char*chp));
int
 waitfor Q((const pid_t pid)),
 strnIcmp P((const char*a,const char*b,size_t l));

#ifdef NOstrcspn
int
 strcspn P((const char*const whole,const char*const sub));
#endif

#define LENoffset	(TABWIDTH*LENtSTOP)
#define MAXfoldlen	(LENoffset-STRLEN(sfolder)-1)
