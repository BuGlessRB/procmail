/*$Id: regexp.h,v 1.10 1994/08/12 17:34:31 berg Exp $*/

struct eps{unsigned opc;struct eps*next;
 union {struct eps*awn;int sopc;} sp;}*
 bregcomp P((const char*const a,unsigned ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,const uchar*str,size_t len,
  const unsigned ign_case));
