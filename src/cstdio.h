/*$Id: cstdio.h,v 1.6 1994/04/08 15:22:20 berg Exp $*/

void
 pushrc P((const char*const name)),
 duprcs P((void)),
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
