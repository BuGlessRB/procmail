/*$Id: regexp.h,v 1.7 1993/05/28 14:40:16 berg Exp $*/

struct eps{unsigned opc;struct eps*next;
 union {struct eps*awn;int sopc;} sp;}*
 bregcomp P((const char*const a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,size_t len,int ign_case));
