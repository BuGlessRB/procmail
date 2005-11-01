/* $Id: lmtp.h,v 1.2 2005/07/13 11:24:59 guenther Exp $ */

extern int childserverpid;
extern char detaildelim;

struct lmtp_rcpt{struct auth_identity*id;char*detail;char*domain;};

struct lmtp_rcpt
 *lmtp P((struct lmtp_rcpt**lrout,char*invoker));
void
 flushoverread P((void)),
 freeoverread P((void)),
 lmtpresponse P((int retcode));
