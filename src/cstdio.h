/*$Id: cstdio.h,v 1.3 1992/10/02 14:39:46 berg Exp $*/

void
 pushrc P((const char*const name)),
 closerc P((void)),
 ungetb P((const x));
int
 poprc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p)),
 getb P((void)),
 testb P((const x)),
 sgetc P((void)),
 skipspace P((void));
