/*$Id: regexp.h,v 1.3 1992/11/11 14:00:49 berg Exp $*/

struct eps{unsigned opc;struct eps*stack,*spawn,*next;}*
 bregcomp P((const char*a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*text,size_t len,int ign_case));
