/*$Id: goodies.h,v 1.1 1992/09/28 14:27:58 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const int sarg)),
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv));
int
 waitfor Q((const pid_t pid));

extern long Stdfilled;

#define TMNATE		'\377'
