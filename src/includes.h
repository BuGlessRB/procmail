/*$Id: includes.h,v 1.24 1993/06/21 14:24:25 berg Exp $*/

#include "../autoconf.h"
#include "../config.h"
	/* not all the "library identifiers" specified here need to be
	   available for all programs in this package; some have substitutes
	   as well (see autoconf); this is just an informal list */

#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE	      /* sad, but needed on HP-UX when compiling -Aa */
#endif

#include <sys/types.h>		/* pid_t mode_t uid_t gid_t off_t */
#ifndef UNISTD_H_MISSING
#include <unistd.h>		/* open() read() write() close() dup() pipe()
				/* fork() getuid() getgid() getpid() execve()
				   execvp() sleep() setuid() setgid()
				   setrgid() chown() */
#endif
#include <stdio.h>		/* setbuf() fclose() stdin stdout stderr
				/* fopen() fread() fwrite() fgetc() getc()
				   fdopen() putc() fputs() FILE EOF */
#ifndef STDDEF_H_MISSING
#include <stddef.h>		/* ptrdiff_t size_t */
#endif
#ifndef STDLIB_H_MISSING
#include <stdlib.h>		/* getenv() malloc() realloc() free()
				/* strtol() exit() */
#endif
#include <time.h>		/* time() ctime() time_t */
#include <fcntl.h>		/* fcntl() struct flock O_RDONLY O_WRONLY
				/* O_APPEND O_CREAT O_EXCL */
#include <grp.h>		/* getgrgid() struct group */
#include <pwd.h>		/* getpwuid() getpwnam() struct passwd */
#ifndef DIRENT_H_MISSING
#include <dirent.h>		/* opendir() readdir() closedir() DIR
				/* struct dirent */
#endif
#ifndef SYS_WAIT_H_MISSING
#include <sys/wait.h>		/* wait() WIFEXITED() WIFSTOPPED()
				/* WEXITSTATUS() */
#endif
#ifndef SYS_UTSNAME_H_MISSING
#include <sys/utsname.h>	/* uname() utsname */
#endif
#include <sys/stat.h>		/* stat() S_ISDIR() S_ISREG() struct stat
				/* chmod() mkdir() */
#include <signal.h>		/* signal() kill() alarm() SIG_IGN SIGHUP
				/* SIGINT SIGQUIT SIGALRM SIGTERM */
#ifndef STRING_H_MISSING
#include <string.h>		/* strcpy() strncpy() strcat() strlen()
				/* strspn() strcspn() strchr() strcmp()
				   strncmp() strpbrk() strstr() memmove() */
#endif
#include <errno.h>		/* EINTR EEXIST ENFILE EACCES EAGAIN */
#ifndef SYSEXITS_H_MISSING
#include <sysexits.h>		/* EX_OK EX_USAGE EX_NOINPUT EX_NOUSER
				/* EX_UNAVAILABLE EX_OSERR EX_OSFILE
				   EX_CANTCREAT EX_IOERR EX_TEMPFAIL EX_NOPERM
				   */
#endif

#ifdef STDLIB_H_MISSING
void*malloc(),*realloc();
const char*getenv();
#endif
#ifdef DIRENT_H_MISSING
#ifndef NDIR_H_MISSING
#include <ndir.h>
#define dirent	direct
#else
#ifndef SYS_NDIR_H_MISSING
#include <sys/ndir.h>
#define dirent	direct
#else
#ifndef SYS_DIR_H_MISSING
#include <sys/dir.h>
#define dirent	direct
#else			  /* due to brain-damaged NeXT sys/dirent.h contents */
#ifndef SYS_DIRENT_H_MISSING	     /* sys/dirent.h must be moved down here */
#include <sys/dirent.h>
#else
/* I give up, I can only hope that your system defines DIR and struct dirent */
#endif
#endif
#endif
#endif
#endif /* DIRENT_H_MISSING */
#ifdef STRING_H_MISSING
#include <strings.h>
#ifndef strchr
char*strchr();
#endif
char*strpbrk();
#endif
#ifdef SYS_UTSNAME_H_MISSING
#define NOuname
#endif
#ifdef SYSEXITS_H_MISSING
		/* Standard exit codes, original list maintained
		   by Eric Allman (eric@berkeley.edu) */
#define EX_OK		0
#define EX_USAGE	64
#define EX_NOINPUT	66
#define EX_NOUSER	67
#define EX_UNAVAILABLE	69
#define EX_OSERR	71
#define EX_OSFILE	72
#define EX_CANTCREAT	73
#define EX_IOERR	74
#define EX_TEMPFAIL	75
#define EX_NOPERM	77
#endif

#if O_SYNC
#else
#undef O_SYNC
#define O_SYNC		0
#endif
#ifndef O_RDONLY
#define O_RDONLY	0
#define O_WRONLY	1
#endif
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif
#ifndef tell
#define tell(fd)	lseek(fd,(off_t)0,SEEK_CUR)
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK	EACCES
#endif
#ifndef EAGAIN
#define EAGAIN		EINTR
#endif

#ifndef EOF
#define EOF	(-1)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)	(((mode)&S_IFMT)==S_IFDIR)
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#endif

#ifndef S_ISREG
#define S_ISREG(mode)	(((mode)&S_IFMT)==S_IFREG)
#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#endif

#ifndef S_ISLNK
#ifndef S_IFLNK
#define lstat(path,stbuf)	stat(path,stbuf)
#define S_ISLNK(mode)	0
#else
#define S_ISLNK(mode)	(((mode)&S_IFMT)==S_IFLNK)
#endif
#endif

#ifndef S_IFMT
#define S_IFMT	0170000
#endif

#ifndef S_IRWXU
#define S_IRWXU 00700
#define S_IRWXG 00070
#define S_IRWXO 00007
#endif
#ifndef S_IWUSR
#ifdef S_IREAD
#define S_IRUSR	 S_IREAD
#define S_IWUSR	 S_IWRITE
#define S_IXUSR	 S_IEXEC
#else
#define S_IRUSR	 0400
#define S_IWUSR	 0200
#define S_IXUSR	 0100
#endif /* S_IREAD */
#define S_IRGRP	 0040
#define S_IWGRP	 0020
#define S_IXGRP	 0010
#define S_IROTH	 0004
#define S_IWOTH	 0002
#define S_IXOTH	 0001
#endif /* S_IWUSR */
#ifndef S_ISGID
#define S_ISUID 04000
#define S_ISGID 02000
#endif

#ifdef WMACROS_NON_POSIX
#ifdef WIFEXITED
#undef WIFEXITED
#endif
#ifdef WIFSTOPPED
#undef WIFSTOPPED
#endif
#ifdef WEXITSTATUS
#undef WEXITSTATUS
#endif
#endif /* WMACROS_NON_POSIX */

#ifndef WIFEXITED
#define WIFEXITED(waitval)	(!((waitval)&255))
#endif
#ifndef WIFSTOPPED
#define WIFSTOPPED(waitval)	(((waitval)&255)==127)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(waitval)	((waitval)>>8&255)
#endif

extern /*const*/char**environ;
extern errno;

#ifndef STDIN_FILENO
#define STDIN	0
#define STDOUT	1
#define STDERR	2
#else
#define STDIN	STDIN_FILENO
#define STDOUT	STDOUT_FILENO
#define STDERR	STDERR_FILENO
#endif

#ifdef NO_fcntl_LOCK
#ifndef NOfcntl_lock
#define NOfcntl_lock
#endif
#endif
#ifdef NO_lockf_LOCK
#ifdef USElockf
#undef USElockf
#endif
#endif
#ifdef NO_flock_LOCK
#ifdef USEflock
#undef USEflock
#endif
#endif

#ifndef NOuname
#ifndef P		  /* SINIX V5.23 has the wrong prototype for uname() */
extern int uname();					 /* so we fix it :-) */
#define Uname(name)		((int(*)(struct utsname*))uname)(name)
#else
#define Uname(name)		uname(name)		    /* no fix needed */
#endif /* P */
#endif /* NOuname */
				 /* NEWS OS 5.X has the wrong prototype here */
#define Fdopen(fd,type)		((FILE*)fdopen(fd,type))

#ifndef strchr		   /* for very old K&R compatible include files with */
#ifdef P						/* new K&R libraries */
#ifdef const
#ifdef void
extern char*strchr();
extern char*strpbrk();
extern char*strstr();
extern void*memmove();
#endif
#endif
#endif
#endif

#define Const			/*const*/     /* Convex cc doesn't grok this */

#ifdef const
#ifndef P				      /* no prototypes without const */
#define P(args) ()
#endif
#endif

#ifdef NOrename
#define rename(old,new) (-(link(old,new)||unlink(old)))
#endif

#ifdef NOsetrgid
#ifdef NOsetregid
#define setrgid(gid)	(-1)
#else
#define setrgid(gid)	setregid(gid,-1)
#endif
#endif

#ifdef NOmkdir
#define mkdir(dir,mode) (-1)
#endif

#ifdef NOmemmove
#define memmove(to,from,count) smemmove(to,from,count)
#endif

#ifndef P
#define P(args)		args
#endif
#define Q(args)		() /* needed until function definitions are ANSI too */

#ifdef oBRAIN_DAMAGE
#undef offsetof
#endif
#ifndef offsetof
#define offsetof(s,m) ((char*)&(((s*)sizeof(s))->m)-(char*)sizeof(s))
#endif

#define PROGID		const char progid[]="Stephen R. van den Berg"
#define maxindex(x)	(sizeof(x)/sizeof((x)[0])-1)
#define STRLEN(x)	(sizeof(x)-1)
#define ioffsetof(s,m)	((int)offsetof(s,m))
#define numeric(x)	((unsigned)((x)-'0')<='9'-'0')

#define mx(a,b)		((a)>(b)?(a):(b))
