/*$Id: regexp.h,v 1.6 1993/05/19 16:47:43 berg Exp $*/

struct eps{unsigned opc;struct eps*next;
  union{struct eps*awn;unsigned sopc;}sp;}*
 bregcomp P((const char*const a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,size_t len,int ign_case));
