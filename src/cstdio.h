/*$Id: cstdio.h,v 1.10 1999/01/29 22:04:55 guenther Exp $*/

void
 pushrc P((const char*const name)),
 duprcs P((void)),
 closerc P((void)),
 ungetb P((const x)),
 skipline P((void));
int
 poprc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p,char*end)),
 getb P((void)),
 testB P((const x)),
 sgetc P((void)),
 skipspace P((void)),
 getlline P((char*target));

extern struct dynstring*incnamed;
