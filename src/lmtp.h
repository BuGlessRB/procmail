/* $Id: lmtp.h,v 1.2 2000/10/25 08:13:18 guenther Exp $ */

extern int childserverpid;

struct auth_identity
 **lmtp P((struct auth_identity***lrout,char*invoker,int privs));
void
 flushoverread P((void)),
 lmtpresponse P((int retcode));
