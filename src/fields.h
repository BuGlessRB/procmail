/*$Id: fields.h,v 1.7 1994/07/26 17:35:15 berg Exp $*/

struct field
 *findf P((const struct field*const p,struct field**ah)),
 **addfield Q((struct field**pointer,const char*const text,
  const size_t totlen)),
 *delfield P((struct field**pointer));
void
 clear_uhead P((struct field*hdr)),
 concatenate P((struct field*const fldp)),
 flushfield P((struct field**pointer)),
 dispfield P((const struct field*p)),
 addbuf P((void));
int
 readhead P((void));
