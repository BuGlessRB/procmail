/*$Id: common.h,v 1.2 1992/09/30 16:23:50 berg Exp $*/

void
 shexec P((char*const*argv)),
 detab P((char*p)),
 ultstr P((int minwidth,unsigned long val,char*dest));
char*
 pstrspn P((const char*whole,const char*const sub));
int
 strnIcmp P((const char*a,const char*b,unsigned l));

#ifdef NOstrcspn
int
 strcspn P((const char*const whole,const char*const sub));
#endif

#define LENoffset	(TABWIDTH*LENtSTOP)
#define MAXfoldlen	(LENoffset-STRLEN(sfolder)-1)
