/*$Id: regexp.h,v 1.5 1993/03/05 14:40:22 berg Exp $*/

struct eps{unsigned opc;struct eps*stack,*spawn,*next;}*
 bregcomp P((const char*const a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*const text,size_t len,int ign_case));
