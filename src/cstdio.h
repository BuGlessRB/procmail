/*$Id: cstdio.h,v 1.9 1995/03/20 14:51:37 berg Exp $*/

void
 pushrc P((const char*const name)),
 duprcs P((void)),
 closerc P((void)),
 ungetb P((const x)),
 skipline P((void)),
 getlline P((char*target));
int
 poprc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p)),
 getb P((void)),
 testB P((const x)),
 sgetc P((void)),
 skipspace P((void));

extern struct dynstring*incnamed;
