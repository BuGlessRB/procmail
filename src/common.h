/*$Id: common.h,v 1.5 1992/11/11 13:58:54 berg Exp $*/

void
 shexec P((const char*const*argv)),
 detab P((char*p)),
 ultstr P((int minwidth,unsigned long val,char*dest));
char*
 pstrspn P((const char*whole,const char*const sub));
int
 strnIcmp P((const char*a,const char*b,size_t l));

#ifdef NOstrcspn
int
 strcspn P((const char*const whole,const char*const sub));
#endif

#define LENoffset	(TABWIDTH*LENtSTOP)
#define MAXfoldlen	(LENoffset-STRLEN(sfolder)-1)
