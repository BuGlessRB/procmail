/*$Id: cstdio.h,v 1.13 1999/10/20 04:53:16 guenther Exp $*/

void
 pushrc P((const char*const name)),
 changerc P((const char*const name)),
 duprcs P((void)),
 closerc P((void)),
 ungetb P((const int x)),
 skipline P((void));
int
 poprc P((void)),
 bopen P((const char*const name)),
 getbl P((char*p,char*end)),
 getb P((void)),
 testB P((const int x)),
 sgetc P((void)),
 skipspace P((void)),
 getlline P((char*target,char*end));

extern struct dynstring*incnamed;
