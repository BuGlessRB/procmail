/*$Id: goodies.h,v 1.3 1992/10/20 15:35:21 berg Exp $*/

void
 readparse P((char*p,int(*const fpgetc)(),const int sarg)),
 sputenv P((const char*const a)),
 primeStdout P((void)),
 retStdout P((char*const newmyenv)),
 postStdout P((void));
int
 waitfor Q((const pid_t pid));

extern long Stdfilled;

#define TMNATE		'\377'
