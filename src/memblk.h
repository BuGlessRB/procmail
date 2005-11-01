typedef struct memblk {
    char*p;						  /* where it starts */
    long len;			 /* current size, not including trailing NUL */
#ifdef USE_MMAP
    off_t filelen;				     /* how long is the file */
    int fd;					   /* file which is mmap()ed */
#endif
} memblk;

typedef char*(read_func_type) P((char*,long,void*));
typedef int(cleanup_func_type) P((memblk*,long*,long,void*));

void							  /* create block of */
 makeblock P((memblk*const mb,const long len)),		     /* given length */
 wrapblock P((memblk*const mb,char*const p,const long len)),   /* put memblk */
		    /* wrapper around an existing allocation, never use mmap */
 freeblock P((memblk*const mb)),			    /* deallocate it */
 lockblock P((memblk*const mb));   /* protect this block from future changes */
int							  /* by this process */
 resizeblock P((memblk*const mb,const long len,		  /* change the size */
  const int nonfatal));		    /* nonfatal means return error if nonmem */
char		      /* dynamically grow a block to fit data as it comes in */
 *read2blk P((memblk*const mb,long*const filledp,read_func_type*read_func,
  cleanup_func_type*cleanup_func,void*data));

#ifdef USE_MMAP
extern int ISprivate;		     /* is themail a private copy or shared? */
#define isprivate	(themail.fd<0||ISprivate)
#define private(x)	(ISprivate=(x))
#else
#define isprivate	1
#define private(x)	1
#endif

extern memblk themail;
