/*$Id: exopen.h,v 1.9 1994/04/05 15:34:22 berg Exp $*/

int
 unique Q((const char*const full,char*p,const mode_t mode,const verbos,
  const chownit)),
 myrename P((const char*const old,const char*const newn)),
 hlink P((const char*const old,const char*const newn));

#define charsSERIAL	4
#define UNIQnamelen	(1+charsSERIAL+HOSTNAMElen+1)
#define bitsSERIAL	(6*charsSERIAL)
#define maskSERIAL	((1L<<bitsSERIAL)-1)
#define rotbSERIAL	2
#define irotbSERIAL	(1L<<bitsSERIAL-rotbSERIAL)
#define mrotbSERIAL	((maskSERIAL&irotbSERIAL-1)+irotbSERIAL)
