/*$Id: acommon.h,v 1.2 2001/06/23 08:18:38 guenther Exp $*/

const char
 *hostname P((void));
char
 *ultoan P((unsigned long val,char*dest));
void
 ultstr P((int minwidth,unsigned long val,char*dest));
