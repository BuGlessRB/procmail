/*$Id: fields.h,v 1.4 1993/04/27 17:33:57 berg Exp $*/

struct field
 *findf P((const struct field*const p,const struct field*hdr)),
 **addfield Q((struct field**pointer,const char*const text,
  const size_t totlen));
void
 concatenate P((struct field*const fldp)),
 renfield Q((struct field**const pointer,const size_t oldl,
  const char*const newname,const size_t newl)),
 flushfield P((struct field**pointer)),
 dispfield P((const struct field*p)),
 addbuf P((void));
int
 readhead P((void));
