/*$Id: misc.h,v 1.10 1993/02/02 15:27:15 berg Exp $*/

void
 elog P((const char*const newt)),
 ignoreterm P((void)),
 writeerr P((const char*const line)),
 progerr P((const char*const line)),
 chderr P((const char*const dir)),
 readerr P((const char*const file)),
 yell P((const char*const a,const char*const b)),
 nlog P((const char*const a)),
 logqnl P((const char*const a)),
 skipped P((const char*const x)),
 sterminate P((void)),
 terminate P((void)),
 suspend P((void)),
 app_val P((struct dyna_long*const sp,const long val)),
 firstchd P((void)),
 srequeue P((void)),
 slose P((void)),
 sbounce P((void)),
 catlim P((const char*src)),
 setdef P((const char*const name,const char*const contents)),
 metaparse P((const char*p)),
 asenvcpy P((char*src)),
 asenv P((const char*const chp)),
 concatenate P((char*p));
int
 forkerr Q((const pid_t pid,const char*const a)),
 nextrcfile P((void)),
 alphanum P((const unsigned c));
char
 *lastdirsep P((const char*filename)),
 *cat P((const char*const a,const char*const b)),
 *tstrdup P((const char*const a)),
 *cstr P((char*const a,const char*const b)),
 *skpspace P((const char*chp)),
 *gobenv P((char*chp)),
 *egrepin P((char*expr,const char*source,const long len,int casesens));
const char
 *tgetenv P((const char*const a));
long
 renvint P((const long i,const char*const env));

extern didchd;

#define MAXvarvals	maxindex(strenvvar)
