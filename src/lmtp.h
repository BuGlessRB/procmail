/* $Id: lmtp.h,v 1.1 1999/12/12 08:50:55 guenther Exp $ */

extern int ctopfd,childserverpid;
extern char*overread;
extern size_t overlen;

struct auth_identity
 **lmtp P((struct auth_identity***lrout,char*invoker,int privs));
void
 lmtpresponse P((int retcode));
