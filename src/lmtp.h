/* $Id$ */

extern int childserverpid;
extern char detaildelim;

struct lmtp_rcpt{struct auth_identity*id;char*detail;char*domain;};

struct lmtp_rcpt
 *lmtp P((struct lmtp_rcpt**lrout,char*invoker));
void
 flushoverread P((void)),
 freeoverread P((void)),
 lmtpresponse P((int retcode));
