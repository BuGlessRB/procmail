/*$Id: regexp.h,v 1.8 1994/05/26 13:48:20 berg Exp $*/

struct eps{unsigned opc;struct eps*next;
 union {struct eps*awn;int sopc;} sp;}*
 bregcomp P((const char*const a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,const uchar*str,size_t len,
  int ign_case));
