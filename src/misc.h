/*$Id: misc.h,v 1.46 2000/10/24 00:16:46 guenther Exp $*/

struct dyna_long{int filled,tspace;union{int i;off_t o;long l;}*vals;};
struct dynstring{struct dynstring*enext;char ename[255];};

#define app_valo(sp,val)	(*(off_t*)app_val_(&sp)=(val))
#define app_vall(sp,val)	(*(long *)app_val_(&sp)=(val))
#define app_vali(sp,val)	(*(int	*)app_val_(&sp)=(val))
#define acc_valo(sp,off)	sp.vals[off].o		/* this is an lvalue */
#define acc_vall(sp,off)	sp.vals[off].l			    /* ditto */
#define acc_vali(sp,off)	sp.vals[off].i			    /* ditto */

void
 elog P((const char*const newt)),
 ignoreterm P((void)),
 shutdesc P((void)),
 checkroot P((const int c,const unsigned long Xid)),
 setids P((void)),
 writeerr P((const char*const line)),
 progerr P((const char*const line,int xitcode,int okay)),
 chderr P((const char*const dir)),
 readerr P((const char*const file)),
 verboff P((void)),
 verbon P((void)),
 newid P((void)),
 zombiecollect P((void)),
 yell P((const char*const a,const char*const b)),
 nlog P((const char*const a)),
 logqnl P((const char*const a)),
 skipped P((const char*const x)),
 onguard P((void)),
 offguard P((void)),
 sterminate P((void)),
 Terminate P((void)),
 suspend P((void)),
 *app_val_ P((struct dyna_long*const sp)),
 srequeue P((void)),
 slose P((void)),
 sbounce P((void)),
 squeeze P((char*target)),
 rcst_nosgid P((void));
int
 forkerr Q((const pid_t pid,const char*const a)),
 nextrcfile P((void)),
 enoughprivs Q((const auth_identity*const passinvk,const uid_t euid,
  const gid_t egid,const uid_t uid,const gid_t gid)),
 conditions P((char flags[],const int prevcond,const int lastsucc,
  const int lastcond,int nrcond));
char
 *tstrdup P((const char*const a)),
 *cstr P((char*const a,const char*const b)),
 *egrepin P((char*expr,const char*source,const long len,int casesens));
const char
 *newdynstring P((struct dynstring**const adrp,const char*const chp));

extern int fakedelivery;
