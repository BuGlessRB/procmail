/* $Id: from.h,v 1.1 2000/10/25 08:13:17 guenther Exp $ */

int
 eqFrom_ P((const char*const a));
const char
 *skipFrom_ P((const char*startchar,long*tobesentp));
void
 makeFrom P((const char*from,const char*const invoker,int privs)),
 lmtpFrom P((const char*const from,const char*const invoker,int privs));
