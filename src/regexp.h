/*$Id: regexp.h,v 1.2 1992/10/02 14:41:07 berg Exp $*/

struct eps{unsigned opc;struct eps*stack,*spawn,*next;}*
 bregcomp P((const char*a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*text,size_t len,int ign_case));
