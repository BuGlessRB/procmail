/*$Id: exopen.h,v 1.5 1993/05/05 13:06:12 berg Exp $*/

const char*
 hostname P((void));
void
 ultoan P((unsigned long val,char*dest));
int
 unique Q((const char*const full,char*const p,const mode_t mode,const verbos)),
 myrename P((const char*const old,const char*const newn));

#define charsSERIAL	4
#define UNIQnamelen	(1+charsSERIAL+HOSTNAMElen+1)
#define bitsSERIAL	(6*charsSERIAL)
#define maskSERIAL	((1L<<bitsSERIAL)-1)
#define rotbSERIAL	2
#define irotbSERIAL	(1L<<bitsSERIAL-rotbSERIAL)
#define mrotbSERIAL	((maskSERIAL&irotbSERIAL-1)+irotbSERIAL)
