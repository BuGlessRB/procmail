/*$Id: hsort.h,v 1.1 1994/04/05 15:34:44 berg Exp $*/

void hsort Q((void*base,size_t nelem,size_t width,
 int(*fcmp)(const void*,const void*)));
