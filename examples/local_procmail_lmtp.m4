divert(0)
VERSIONID(`@(#)local_procmail_lmtp.m4	     1.0 1999/12/15')
divert(-1)

define(`LOCAL_MAILER_PATH',
	ifelse(_ARG_, `',
		ifdef(`PROCMAIL_MAILER_PATH',
			PROCMAIL_MAILER_PATH,
			`/usr/local/bin/procmail.lmtp'),
		_ARG_))
define(`LOCAL_MAILER_FLAGS', `SPhn9Xzm')
define(`LOCAL_MAILER_ARGS', `procmail -Y -a $h -z')
