/*$Id: common.h,v 1.13 2000/11/27 07:09:21 guenther Exp $*/

void
 shexec P((const char*const*argv))  __attribute__((noreturn)),
 detab P((char*p)),
 ultstr P((int minwidth,unsigned long val,char*dest));
char
 *skpspace P((const char*chp));
int
 waitfor Q((const pid_t pid));

#ifdef NOstrcspn
int
 strcspn P((const char*const whole,const char*const sub));
#endif

#ifdef NOstrncasecmp
int
 strncasecmp Q((const char*a,const char*b,size_t l));
#endif

#define LENoffset	(TABWIDTH*LENtSTOP)
#define MAXfoldlen	(LENoffset-STRLEN(sfolder)-1)

#define NO_PROCESS	(-256)
