/*$Id: pipes.h,v 1.13 2000/10/24 00:16:48 guenther Exp $*/

struct memblk;					       /* predeclare the tag */
void
 inittmout P((const char*const progname)),
 ftimeout P((void)),
 resettmout P((void)),
 exectrap P((const char*const tp));
int
 pipthrough P((char*line,char*source,const long len));
long
 pipin P((char*const line,char*source,long len));
char
 *readdyn P((struct memblk*const mb,long*const filled,long oldfilled)),
 *fromprog Q((char*name,char*const dest,size_t max));

extern int setxit;
extern pid_t pidchild;
extern volatile time_t alrmtime;
extern volatile int toutflag;
extern int pipw;
