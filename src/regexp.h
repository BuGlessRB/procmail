/*$Id: regexp.h,v 1.12 1994/09/13 19:13:01 berg Exp $*/

struct eps
{ unsigned opc;struct eps*next;
  union seps {struct eps*awn;int sopc;} sp;
}*
 bregcomp P((const char*const a,const unsigned ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,const uchar*str,size_t len,
  const unsigned ign_case));
