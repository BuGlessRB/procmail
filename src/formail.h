/*$Id: formail.h,v 1.4 1993/04/27 17:34:03 berg Exp $*/

#define BSIZE		128

#define NAMEPREFIX	"formail: "
#define HEAD_DELIMITER	':'

#define Re		(re+1)
#define putssn(a,l)	tputssn(a,(size_t)(l))
#define putcs(a)	(errout=putc(a,mystdout))
#define lputssn(a,l)	ltputssn(a,(size_t)(l))
#define PRDO		poutfd[0]
#define PWRO		poutfd[1]
#define FLD_HEADSIZ	((size_t)offsetof(struct field,fld_text[0]))

struct saved{const char*const headr;const int lenr;int rexl;char*rexp;};

extern const char binsh[],sfolder[],couldntw[];
extern errout,oldstdout,quiet,buflast;
extern pid_t child;
extern FILE*mystdout;
extern size_t nrskip,nrtotal,buflen,buffilled;
extern long totallen;
extern char*buf,*logsummary;

extern struct field{size_t id_len;size_t tot_len;struct field*fld_next;
 char fld_text[255];}*rdheader,*xheader,*Xheader;

int
 eqFrom_ P((const char*const a)),
 breakfield Q((const char*const line,size_t len));
