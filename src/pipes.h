/*$Id: pipes.h,v 1.9 1994/05/26 14:13:20 berg Exp $*/

void
 inittmout P((const char*const progname)),
 ftimeout P((void)),
 resettmout P((void)),
 exectrap P((const char*const tp));
int
 pipthrough P((char*line,char*source,const long len));
long
 pipin P((char*const line,char*source,long len));
char*
 readdyn P((char*bf,long*const filled)),
 *fromprog Q((char*name,char*const dest,size_t max));

#define PRDO	poutfd[0]
#define PWRO	poutfd[1]
#define PRDI	pinfd[0]
#define PWRI	pinfd[1]
#define PRDB	pbackfd[0]
#define PWRB	pbackfd[1]

extern const char exitcode[];
extern int setxit;
extern pid_t pidchild;
extern volatile time_t alrmtime;
extern volatile toutflag;
extern int pipw;
