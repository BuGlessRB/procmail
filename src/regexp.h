/*$Id: regexp.h,v 1.1 1992/09/28 14:27:58 berg Exp $*/

struct eps{unsigned opc;struct eps*stack,*spawn,*next;}*
 bregcomp P((const char*a,int ign_case));
char*
 bregexec Q((struct eps*code,const uchar*text,size_t len,int ign_case));
