/*$Id: fields.h,v 1.3 1992/11/11 13:59:34 berg Exp $*/

struct field
 *findf P((const struct field*const p,const struct field*hdr)),
 **addfield Q((struct field**pointer,const char*const text,
  const size_t totlen));
void
 concatenate P((struct field*const fldp)),
 renfield Q((struct field**const pointer,const size_t oldl,
  const char*const newname,const size_t newl)),
 clearfield P((struct field**pointer)),
 flushfield P((struct field**pointer)),
 dispfield P((const struct field*p)),
 addbuf P((void));
int
 readhead P((void));
