/*$Id: formisc.h,v 1.1 1992/09/28 14:28:05 berg Exp $*/

void
 *tmalloc Q((const size_t len)),
 *trealloc Q((void*old,const size_t len)),
 tfree P((void*a)),
 loadsaved P((const struct saved*const sp)),
 loadbuf Q((const char*const text,const size_t len)),
 loadchar P((const c)),
 log P((const char*const a)),
 tputssn Q((const char*a,size_t l)),
 ltputssn Q((const char*a,size_t l)),
 lputcs P((const i)),
 startprog P((const char*const*const argv)),
 nofild P((void)),
 waitforit P((void)),
 nlog P((const char*const a)),
 logqnl P((const char*const a)),
 closemine P((void)),
 opensink P((void));
char*
 skipwords P((const char*start,const char*const end));
int
 getline P((void)),
 mystrstr P((const char*whole,const char*const part,const char*end));
