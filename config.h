/*$Id: config.h,v 1.53 1994/06/30 12:01:25 berg Exp $*/

/*#define sMAILBOX_SEPARATOR	"\1\1\1\1\n"	/* sTART- and eNDing separ.  */
/*#define eMAILBOX_SEPARATOR	"\1\1\1\1\n"	/* uncomment (one or both)
						   if your mail system uses
	nonstandard mail separators (non sendmail or smail compatible mailers
	like MMDF), if yours is even different, uncomment and change the
	value of course */

/* KEEPENV and PRESTENV should be defined as a comma-separated null-terminated
   list of strings */

/* every environment variable appearing in KEEPENV will not be thrown away
 * upon startup of procmail, e.g. you could define KEEPENV as follows:
 * #define KEEPENV	{"TZ","LANG",0}
 * environment variables ending in an _ will designate the whole group starting
 * with this prefix (e.g. "LC_").
 */
#define KEEPENV		{"TZ",0}

/* every environment variable appearing in PRESTENV will be set or wiped
 * out of the environment (variables without an '=' sign will be thrown
 * out), e.g. you could define PRESTENV as follows:
 * #define PRESTENV	{"IFS","ENV","PWD","PATH=$HOME/bin:/bin:/usr/bin",0}
 * any side effects (like setting the umask after an assignment to UMASK) will
 * *not* take place
 */
#define PRESTENV	{"IFS","ENV","PWD","PATH=$HOME/bin:/bin:/usr/bin", \
			 "USER=$LOGNAME",0}

/************************************************************************
 * Only edit below this line if you have viewed/edited this file before *
 ************************************************************************/

/* every user & group appearing in TRUSTED_IDS is allowed to use the -f option
   if the list is empty (just a terminating 0), everyone can use it
   TRUSTED_IDS should be defined as a comma-separated null-terminated
   list of strings;  if unauthorised users use the -f option, an extra
   >From_ field will be added in the header */

#define TRUSTED_IDS	{"root","daemon","uucp","mail","x400","network",\
			 "list","lists","news",0}

/*#define NO_USER_TO_LOWERCASE_HACK	/* uncomment if your getpwnam() is
					   case insensitive or if procmail
	will always be supplied with the correct case in the explicit
	delivery mode argument(s) */

/*#define NO_fcntl_LOCK		/* uncomment any of these three if you	     */
/*#define NO_lockf_LOCK		/* definitely do not want procmail to make   */
/*#define NO_flock_LOCK		/* use of those kernel-locking methods	     */

/*#define NO_NFS_ATIME_HACK	/* uncomment if you're definitely not using
				   NFS mounted filesystems and can't afford
	procmail to sleep for 1 sec. before writing a mailbox */

/*#define SYSTEM_MBOX	"$HOME/.mail"	/* uncomment and/or change if the
					   preset default mailbox is *not*
	suitable or if you want standard mail delivery to take place in a
	different file from the normal mail-spool-file.
	(it will supersede the value of SYSTEM_MAILBOX in autoconf.h) */

/*#define DEFsendmail	"/bin/mail"	/* uncomment and/or change if the
					   autoconfigured default SENDMAIL is
	not suitable */

#define ETCRC	"/etc/procmailrc"	/* optional global procmailrc startup
					   file (will only be read if procmail
	is started with no rcfile on the command line). */

#define ETCRCS	"/etc/procmailrcs/"	/* optional trusted path prefix for
					   rcfiles which will be executed with
	the uid of the owner of the rcfile (this only happens if procmail is
	called with the -m option, without variable assignments on the command
	line). */

/*#define console	"/dev/console"	/* uncomment if you want procmail to
					   use the console (or any other
	terminal or file) to print any error messages that could not be dumped
	in the "logfile"; only recommended for debugging purposes, if you have
	trouble creating a "logfile" or suspect that the trouble starts before
	procmail can interpret any rcfile or arguments. */

/************************************************************************
 * Only edit below this line if you *think* you know what you are doing *
 ************************************************************************/

#define ROOT_uid	0

#define UPDATE_MASK	S_IXOTH	   /* bit set on mailboxes when mail arrived */
#define OVERRIDE_MASK	(S_IXUSR|S_ISUID|S_ISGID|S_ISVTX)    /* if found set */
		    /* the permissions on the mailbox will be left untouched */
#define INIT_UMASK	(S_IRWXG|S_IRWXO)			   /* == 077 */
#define NORMperm	\
 (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH|UPDATE_MASK)
	     /* == 0667, normal mode bits used to create files, before umask */
#define READperm	(S_IRUSR|S_IRGRP|S_IROTH)		  /* == 0444 */
#define NORMdirperm	(S_IRWXU|S_IRWXG|S_IRWXO)		  /* == 0777 */
#define LOCKperm	READperm  /* mode bits used while creating lockfiles */
#define MAX_locksize	16	  /* lockfiles are expected not to be longer */
#ifndef SMALLHEAP
#define DEFlinebuf	2048		 /* default max expanded line length */
#define BLKSIZ		16384		  /* blocksize while reading/writing */
#define STDBUF		1024		     /* blocksize for emulated stdio */
#else		   /* and some lower defaults for the unfortunate amongst us */
#define DEFlinebuf	512
#define BLKSIZ		1024
#define STDBUF		128
#endif /* SMALLHEAP */
#define FAKE_FIELD	">From "
#define HOSTNAMElen	9	  /* determines hostname-ID-len on tempfiles */
#define BOGUSprefix	"BOGUS."	     /* prepended to bogus mailboxes */
#define PROCMAILRC	".procmailrc"
#define DEFsuspend	16		 /* multi-purpose 'idle loop' period */
#define DEFlocksleep	8
#define TOkey		"^TO"
#define TOsubstitute	"(^((Original-)?(Resent-)?(To|Cc|Bcc)|\
(X-Envelope|Apparently(-Resent)?)-To):(.*[^a-zA-Z])?)"
#define FROMDkey	"^FROM_DAEMON"		     /* matches most daemons */
#define FROMDsubstitute "(^(Precedence:.*(junk|bulk|list)|\
(((Resent-)?(From|Sender)|X-Envelope-From):|>?From )(.*[^(.%@a-z0-9])?(\
Post(ma?(st(e?r)?|n)|office)|Mail(er)?|daemon|mmdf|root|uucp|LISTSERV|owner|\
request|bounce|serv(ices?|er)|Admin(istrator)?)([^).!:a-z0-9].*)?$[^>]))"
#define FROMMkey	"^FROM_MAILER"	      /* matches most mailer-daemons */
#define FROMMsubstitute "(^(((Resent-)?(From|Sender)|X-Envelope-From):|\
>?From )(.*[^(.%@a-z0-9])?(Post(ma(st(er)?|n)|office)|Mail(er)?|daemon|mmdf|\
root|uucp|serv(ices?|er)|Admin(istrator)?)([^).!:a-z0-9].*)?$[^>])"
#define DEFshellmetas	"&|<>~;?*["		    /* never put '$' in here */
#define DEFmaildir	"$HOME"
#define DEFdefault	"$ORGMAIL"
#define DEFmsgprefix	"msg."
#define DEFlockext	".lock"
#define DEFshellflags	"-c"
#define DEFlocktimeout	1024		     /* defaults to about 17 minutes */
#define DEFtimeout	(DEFlocktimeout-64)	   /* 64 seconds to clean up */
#define DEFnoresretry	4      /* default nr of retries if no resources left */
#define nfsTRY		(7+1) /* nr of times+1 to ignore spurious NFS errors */
#define DEFlogabstract	-1    /* abstract by default, but don't mail it back */
#define COMSAThost	"localhost"    /* where the biff/comsat daemon lives */
#define COMSATservice	"biff"	    /* the service name of the comsat daemon */
#define COMSATprotocol	"udp" /* if you change this, comsat() needs patching */
#define COMSATxtrsep	":"		 /* mailbox-spec extension separator */
#define SERV_ADDRsep	'@'	      /* when overriding in COMSAT=serv@addr */
#define DEFcomsat	"no"		/* when an rcfile has been specified */

#define BinSh		"/bin/sh"
#define RootDir		"/"
#define DevNull		"/dev/null"
#define NICE_RANGE	39			  /* maximal nice difference */
#define chCURDIR	'.'			    /* the current directory */
#define chPARDIR	".."			     /* the parent directory */
#define DIRSEP		"/"		 /* directory separator symbols, the */
				   /* last one should be the most common one */

#define EOFName		" \t\n#`'\");"

#define HELPOPT1	'h'		 /* options to get command line help */
#define HELPOPT2	'?'

#define VERSIONOPT	'v'			/* option to display version */
#define PRESERVOPT	'p'			     /* preserve environment */
#define TEMPFAILOPT	't'		      /* return EX_TEMPFAIL on error */
#define MAILFILTOPT	'm'	     /* act as a general purpose mail filter */
#define FROMWHOPT	'f'			   /* set name on From_ line */
#define REFRESH_TIME	'-'		     /* when given as argument to -f */
#define ALTFROMWHOPT	'r'		/* alternate and obsolete form of -f */
#define OVERRIDEOPT	'o'		     /* do not generate >From_ lines */
#define ARGUMENTOPT	'a'					   /* set $1 */
#define DELIVEROPT	'd'		  /* deliver mail to named recipient */
#define PM_USAGE	\
 "Usage: procmail [-vpto] [-f fromwhom] [parameter=value | rcfile] ...\
\n   Or: procmail [-to] [-f fromwhom] [-a argument] -d recipient ...\
\n   Or: procmail [-pt] -m [parameter=value] ... rcfile mail_from rcpt_to ...\
\n"
#define PM_HELP		\
 "\t-v\t\tdisplay the version number and exit\
\n\t-p\t\tpreserve (most of) the environment upon startup\
\n\t-t\t\tfail softly if mail is undeliverable\
\n\t-f fromwhom\t(re)generate the leading 'From ' line\
\n\t-o override the leading 'From ' line if necessary\
\n\t-a argument\twill set $1\
\n\t-d recipient\texplicit delivery mode\
\n\t-m\t\tact as a general purpose mail filter\n"
#define PM_QREFERENCE	\
 "Recipe flag quick reference:\
\n\tH  egrep header (default)\tB  egrep body\
\n\tD  distinguish case\
\n\tA  also execute this recipe if the common condition matched\
\n\ta  same as 'A', but only if the previous recipe was successful\
\n\tE  else execute this recipe, if the preceding condition didn't match\
\n\te  on error execute this recipe, if the previous recipe failed\
\n\th  deliver header (default)\tb  deliver body (default)\
\n\tf  filter\t\t\ti  ignore write errors\
\n\tc  continue with the next recipe in any case\
\n\tw  wait for a filter or program\
\n\tW  same as 'w', but suppress 'Program failure' messages\n"

#define MINlinebuf	128    /* minimal LINEBUF length (don't change this) */
#define FROM_EXPR	"\nFrom "
#define FROM		"From "
#define SHFROM		"From"
#define NSUBJECT	"^Subject:.*$"
#define MAXSUBJECTSHOW	78
#define FOLDER		"  Folder: "
#define LENtSTOP	9 /* tab stop at which message length will be logged */

#define TABCHAR		"\t"
#define TABWIDTH	8

#define RECFLAGS	"HBDAahbfcwWiEe"
#define HEAD_GREP	 0
#define BODY_GREP	  1
#define DISTINGUISH_CASE   2
#define ALSO_NEXT_RECIPE    3
#define ALSO_N_IF_SUCC	     4
#define PASS_HEAD	      5
#define PASS_BODY	       6
#define FILTER			7
#define CONTINUE		 8
#define WAIT_EXIT		  9
#define WAIT_EXIT_QUIET		   10
#define IGNORE_WRITERR		    11
#define ELSE_DO			     12
#define ERROR_DO		      13

#define UNIQ_PREFIX	'_'	  /* prepended to temporary unique filenames */
#define ESCAP		">"

		/* some formail-specific configuration options: */

#define UNKNOWN		"foo@bar"	  /* formail default originator name */
#define OLD_PREFIX	"Old-"			 /* formail field-Old-prefix */
#define BABYL_SEP1	'\037'		       /* BABYL format separator one */
#define BABYL_SEP2	'\f'		       /* BABYL format separator two */
#define DEFfileno	"FILENO=000"		/* split counter for formail */
#define LEN_FILENO_VAR	7			       /* =strlen("FILENO=") */
#define CHILD_FACTOR	3/4 /* do not parenthesise; average running children */

#define FM_SKIP		'+'		      /* skip the first nnn messages */
#define FM_TOTAL	'-'	    /* only spit out a total of nnn messages */
#define FM_BOGUS	'b'			 /* leave bogus Froms intact */
#define FM_QPREFIX	'p'			  /* define quotation prefix */
#define FM_CONCATENATE	'c'	      /* concatenate continued header-fields */
#define FM_FORCE	'f'   /* force formail to accept an arbitrary format */
#define FM_REPLY	'r'		    /* generate an auto-reply header */
#define FM_KEEPB	'k'		   /* keep the header, when replying */
#define FM_TRUST	't'	/* trust the sender to supply a valid header */
#define FM_LOGSUMMARY	'l'    /* generate a procmail-compatible log summary */
#define FM_SPLIT	's'				      /* split it up */
#define FM_NOWAIT	'n'		      /* don't wait for the programs */
#define FM_EVERY	'e'	/* don't require empty lines leading headers */
#define FM_MINFIELDS	'm'    /* the number of fields that have to be found */
#define DEFminfields	2	    /* before a header is recognised as such */
#define FM_DIGEST	'd'				 /* split up digests */
#define FM_BABYL	'B'		/* split up BABYL format rmail files */
#define FM_QUIET	'q'					 /* be quiet */
#define FM_DUPLICATE	'D'		/* return success on duplicate mails */
#define FM_EXTRACT	'x'			   /* extract field contents */
#define FM_EXTRC_KEEP	'X'				    /* extract field */
#define FM_ADD_IFNOT	'a'		 /* add a field if not already there */
#define FM_ADD_ALWAYS	'A'		       /* add this field in any case */
#define FM_REN_INSERT	'i'			/* rename and insert a field */
#define FM_DEL_INSERT	'I'			/* delete and insert a field */
#define FM_FIRST_UNIQ	'u'		    /* preserve the first occurrence */
#define FM_LAST_UNIQ	'U'		     /* preserve the last occurrence */
#define FM_ReNAME	'R'				   /* rename a field */
#define FM_USAGE	"\
Usage: formail [-bcfrktq] [-D nnn idcache] [-p prefix] [-l folder]\n\
\t[-xXaAiIuU field] [-R ofield nfield]\n\
   Or: formail [+nnn] [-nnn] [-bcfrktnedqB] [-D nnn idcache] [-p prefix]\n\
\t[-m nnn] [-l folder] [-xXaAiIuU field] [-R ofield nfield]\n\
\t-s [prg [arg ...]]\n"
#define FM_HELP		\
 " -b\t\tdon't escape bogus mailbox headers\
\n -c\t\tconcatenate continued header-fields\
\n -f\t\tforce formail to pass along any non-mailbox format\
\n -r\t\tgenerate an auto-reply header, preserve fields with -i\
\n -k\t\ton auto-reply keep the body, prevent escaping with -b\
\n -t\t\ttrust the sender for his return address\
\n -l folder\tgenerate a procmail-compatible log summary\
\n -D nnn idcache\tdetect duplicates with an idcache of length nnn\
\n -s prg arg\tsplit the mail, startup prg for every message\
\n +nnn\t\tskip the first nnn\t-nnn\toutput at most nnn messages\
\n -n\t\tdon't serialise splits\t-e\tempty lines are optional\
\n -d\t\taccept digest format\t-B\texpect BABYL rmail format\
\n -q\t\tbe quiet\t\t-p prefix\tquotation prefix\
\n -m nnn \tmin fields threshold (default 2) for start of message\
\n -x field\textract contents\t-X field\textract fully\
\n -a field\tadd if not present\t-A field\tadd in any case\
\n -i field\trename and insert\t-I field\tdelete and insert\
\n -u field\tfirst unique\t\t-U field\tlast unique\
\n -R oldfield newfield\trename\n"
