/*$Id: cstdio.h,v 1.1 1992/09/28 14:27:59 berg Exp $*/

void
 pushrc P((const char*const name)),
 ungetb P((const x));
int
 poprc P((void)),
 closerc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p)),
 getb P((void)),
 testb P((const x)),
 sgetc P((void)),
 skipspace P((void));
