/*$Id: misc.h,v 1.3 1992/10/02 14:40:44 berg Exp $*/

void
 elog P((const char*const newt)),
 ignoreterm P((void)),
 writeerr P((const char*const line)),
 progerr P((const char*const line)),
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
 catlim Q((char*dest,char*src,size_t lim)),
 setdef P((const char*const name,const char*const contents)),
 asenvcpy P((char*src)),
 asenv P((char*chp)),
 concatenate P((char*p));
int
 forkerr P((const pid_t pid,const char*const a)),
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
