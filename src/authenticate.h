/*$Id: authenticate.h,v 1.1 1997/01/01 14:52:51 srb Exp $*/

/* Generic authentication interface, substitute a suitable module to
   accomodate arbitrary other authentication databases */

typedef struct auth_identity auth_identity;

const auth_identity
 *auth_finduser P((char*const user,const sock)),
 *auth_finduid P((const uid_t uid,const sock));
int
 auth_checkpassword P((const auth_identity*const pass,const char*const pw,
  const allowemptypw));
const char
 *auth_getsecret P((const auth_identity*const pass)),
 *auth_mailboxname P((auth_identity*const pass)),
 *const auth_mailboxinfo P((void)),
 *auth_homedir P((const auth_identity*const pass)),
 *auth_username P((const auth_identity*const pass));
uid_t
 auth_whatuid P((const auth_identity*const pass)),
 auth_whatgid P((const auth_identity*const pass));
void
 auth_end P((void));
