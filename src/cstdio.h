/*$Id: cstdio.h,v 1.2 1992/09/30 17:55:35 berg Exp $*/

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
