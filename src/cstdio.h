/*$Id: cstdio.h,v 1.4 1992/11/11 13:59:08 berg Exp $*/

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
