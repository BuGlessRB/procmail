/*$Id: cstdio.h,v 1.5 1992/11/24 15:59:58 berg Exp $*/

void
 pushrc P((const char*const name)),
 closerc P((void)),
 ungetb P((const x)),
 getlline P((char*target));
int
 poprc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p)),
 getb P((void)),
 testb P((const x)),
 sgetc P((void)),
 skipspace P((void));
