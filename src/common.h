/*$Id: common.h,v 1.6 1993/08/20 11:22:37 berg Exp $*/

void
 shexec P((const char*const*argv)),
 detab P((char*p)),
 ultstr P((int minwidth,unsigned long val,char*dest));
char*
 pstrspn P((const char*whole,const char*const sub));
int
 waitfor Q((const pid_t pid)),
 strnIcmp P((const char*a,const char*b,size_t l));

#ifdef NOstrcspn
int
 strcspn P((const char*const whole,const char*const sub));
#endif

#define LENoffset	(TABWIDTH*LENtSTOP)
#define MAXfoldlen	(LENoffset-STRLEN(sfolder)-1)
