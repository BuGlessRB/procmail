/*$Id: misc.h,v 1.48 2000/10/27 22:07:27 guenther Exp $*/

struct dyna_array{int filled,tspace;char*vals;};
struct dynstring{struct dynstring*enext;char ename[255];};

#define app_val_type(sp,type)	(type*)app_val_(&sp,sizeof(type))
#define app_valo(sp,val)	(*app_val_type(sp,off_t)=(val))
#define app_vall(sp,val)	(*app_val_type(sp,long)=(val))
#define app_vali(sp,val)	(*app_val_type(sp,int)=(val))
#define app_valp(sp,val)	(*app_val_type(sp,const char*)=(val))

#define acc_val_(sp,type,off)	((type*)sp.vals)[off]
#define acc_valo(sp,off)	acc_val_(sp,off_t,off)	/* these are lvalues */
#define acc_vall(sp,off)	acc_val_(sp,long,off)
#define acc_vali(sp,off)	acc_val_(sp,int,off)
#define acc_valp(sp,off)	acc_val_(sp,const char*,off)

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
 *app_val_ P((struct dyna_array*const sp,int size)),
 srequeue P((void)),
 slose P((void)),
 sbounce P((void)),
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
