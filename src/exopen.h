/*$Id: exopen.h,v 1.2 2002/06/30 06:24:09 guenther Exp $*/

int
 unique Q((char*const full,char*p,const size_t len,const mode_t mode,
  const int verbos,const int flags)),
 myrename P((const char*const old,const char*const newn)),
 rlink P((const char*const old,const char*const newn,struct stat*st)),
 hlink P((const char*const old,const char*const newn));

#define UNIQnamelen	30	 /* require how much space as a first guess? */
#define MINnamelen	14		      /* cut to this on ENAMETOOLONG */

#define doCHOWN		1
#define doCHECK		2
#define doLOCK		4
#define doFD		8
#define doMAILDIR	16
