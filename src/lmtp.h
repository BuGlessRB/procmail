/* $Id: lmtp.h,v 1.3 2000/10/27 20:04:01 guenther Exp $ */

extern int childserverpid;

struct auth_identity
 **lmtp P((struct auth_identity***lrout,char*invoker));
void
 flushoverread P((void)),
 lmtpresponse P((int retcode));
