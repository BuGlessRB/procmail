/*$Id: exopen.h,v 1.15 1999/04/02 19:04:58 guenther Exp $*/

int
 unique Q((const char*const full,char*p,const mode_t mode,const verbos,
  const chownit)),
 myrename P((const char*const old,const char*const newn)),
 rlink P((const char*const old,const char*const newn,struct stat*st)),
 hlink P((const char*const old,const char*const newn));

#define charsSERIAL	4
#define UNIQnamelen	(1+charsSERIAL+1+HOSTNAMElen+1)
#define bitsSERIAL	(6*charsSERIAL)
#define maskSERIAL	((1L<<bitsSERIAL)-1)
#define rotbSERIAL	2
#define irotbSERIAL	(1L<<bitsSERIAL-rotbSERIAL)
#define mrotbSERIAL	((maskSERIAL&irotbSERIAL-1)+irotbSERIAL)

#define doCHOWN		1
#define doCHECK		2
#define doLOCK		4
#define doFD		8
