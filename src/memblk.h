typedef struct memblk {
    char*p;						  /* where it starts */
    long len;					 /* currently allocated size */
#ifdef USE_MMAP
    long filelen;				     /* how long is the file */
    int fd;					   /* file which is mmap()ed */
#endif
} memblk;

void
 makeblock P((memblk*const,const long)), /* create block of the given length */
 freeblock P((memblk*const));				    /* deallocate it */
int
 resizeblock P((memblk*,const long,const int)); /* change the allocated size */

#ifdef USE_MMAP
extern int ISprivate;		     /* is themail a private copy or shared? */
#define isprivate	(themail.fd<0||ISprivate)
#define private(x)	(ISprivate=(x))
#else
#define isprivate	1
#define private(x)	1
#endif

extern memblk themail;
