/*$Id: exopen.h,v 1.17 2000/09/28 01:23:17 guenther Exp $*/

int
 unique Q((const char*const full,char*p,const size_t len,const mode_t mode,
  const int verbos,const int chownit)),
 myrename P((const char*const old,const char*const newn)),
 rlink Q((const char*const old,const char*const newn,struct stat*st)),
 hlink P((const char*const old,const char*const newn));

#define UNIQnamelen	24	 /* require how much space as a first guess? */
#define MINnamelen	14		      /* cut to this on ENAMETOOLONG */

#define doCHOWN		1
#define doCHECK		2
#define doLOCK		4
#define doFD		8
