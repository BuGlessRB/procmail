/*$Id: fields.h,v 1.6 1994/05/26 14:12:39 berg Exp $*/

struct field
 *findf P((const struct field*const p,struct field**ah)),
 **addfield Q((struct field**pointer,const char*const text,
  const size_t totlen)),
 *delfield P((struct field**pointer));
void
 clear_uhead P((struct field*hdr)),
 concatenate P((struct field*const fldp)),
 renfield Q((struct field**const pointer,const size_t oldl,
  const char*const newname,const size_t newl)),
 flushfield P((struct field**pointer)),
 dispfield P((const struct field*p)),
 addbuf P((void));
int
 readhead P((void));
