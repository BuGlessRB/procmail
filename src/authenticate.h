/*$Id: authenticate.h,v 1.2 1997/04/02 03:15:37 srb Exp $*/

/* Generic authentication interface, substitute a suitable module to
   accomodate arbitrary other authentication databases */

typedef struct auth_identity auth_identity;

/*const*/auth_identity
 *auth_finduser P((char*const user,const sock)),
 *auth_finduid P((const uid_t uid,const sock));
auth_identity
 *auth_newid P((void));
int
 auth_checkpassword P((const auth_identity*const pass,const char*const pw,
  const allowemptypw)),
 auth_filledid P((const auth_identity*pass));
const char
 *auth_getsecret P((const auth_identity*const pass)),
 *auth_mailboxname P((auth_identity*const pass)),
 *const auth_mailboxinfo P((void)),
 *auth_homedir P((const auth_identity*const pass)),
 *auth_shell P((const auth_identity*const pass)),
 *auth_username P((const auth_identity*const pass));
uid_t
 auth_whatuid P((const auth_identity*const pass)),
 auth_whatgid P((const auth_identity*const pass));
void
 auth_copyid P((auth_identity*newpass,const auth_identity*oldpass)),
 auth_freeid P((auth_identity*pass)),
 auth_end P((void));
