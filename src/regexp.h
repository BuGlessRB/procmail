/*$Id: regexp.h,v 1.4 1992/12/01 15:46:45 berg Exp $*/

struct eps{unsigned opc;struct eps*stack,*spawn,*next;}*
 bregcomp P((const char*a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,size_t len,int ign_case));
