#! /bin/sh
: &&O='cd .' || exec /bin/sh "$0" $argv:q # we're in a csh, feed myself to sh
$O || exec /bin/sh "$0" "$@"		  # we're in a buggy zsh
#$Id: install.sh,v 1.56 1998/11/06 05:35:21 guenther Exp $

umask 022				# set it to a sane value

if test -z "$IFS"
then IFS=" \
	\

"
  export IFS
fi

SHELL=/bin/sh				# make sure we get a decent shell
test -z "$MAKE" && MAKE=make		# provide a default "make"
export SHELL MAKE
umask 022				# making sure that umask has sane value

( exec 2>/dev/null
  case `echo x x` in
     *) exit 0 ;;
  esac
)
if test $? != 0				# for BSDI /bin/sh shells
then
  echo "This shell is not sufficiently Bourne shell compatible."
  echo "I recommend upgrading your /bin/sh first."
  exit 69
fi

test $# = 0 -a -f asked.patch && set dummy `cat asked.patch` && shift

test $# != 1 -a $# != 2 && echo "Usage: install.sh target-directory [.bin]" &&
 exit 64

target="$1"
bindir="$2"

case "$target" in		# Make sure $target is absolute
  /*) ;;
  *) target=`cd "$target";pwd` ;;
esac

test -z "$bindir" && bindir=.bin

test ! -d "$target" && echo "Please create the target directory first" &&
 echo "Make sure it has the right owner & group" && exit 2

cd "`dirname $0`"
PATH=.:$PATH

setid=../src/setid

if test ! -f ../config.h
then
  echo "You must merge the source trees of procmail and SmartList"
  echo "together.  Simply unpack them on top of each other."
  exit 2
fi

if binmail=`procmail -m DEFAULT=/dev/null 'LOG=$SENDMAIL' /dev/null \
  </dev/null 2>&1`
then
  case "$binmail" in
     /*) ;;		# some !@#$%^&*() shells don't grok [!/]
     *) binmail="";;
  esac
  case "$binmail" in
     ""|*procmail:*)
	 echo "Failed in extracting the value of SENDMAIL from procmail"
	 echo \
	"Please make sure that the NEW version of procmail has been installed"
	 echo \
       'If you already have, make sure that "console" is undefined in config.h'
	 echo "This is what I'm seeing:"
	 procmail -v 2>&1 | sed -e 1q
	 exit 64 ;;
  esac
else
  echo "Please make sure that procmail is on our PATH"
  exit 64
fi

if expr "X$bindir" : 'X\.bin' >/dev/null
then
:
else
  echo "I prefer a bin directory that starts with .bin"
  echo "If you want to enforce a different name, patch install.sh first :-)."
  exit 64
fi

export target bindir binmail PATH

if test -f "$target/.etc/rc.init.dist" -a ! -f asked.patch
then
   ( exec 2>/dev/null
     diff >/dev/null
     if test $? = 2
     then
	exec patch </dev/null
     fi
     exit 1
   )
  if test $? != 0
  then
     echo 1>&2 "Although not mandatory, it could save *you* some time if the"
     echo 1>&2 "'diff' and 'patch' utilities are in the PATH.  If either one"
     echo 1>&2 "is not available, forget I asked and run install.sh again."
     echo "$*" >asked.patch
     exit 75
  fi
fi

if test ! -z "$LD_LIBRARY_PATH"
then
  echo '***************************** WARNING *********************************'
  echo '* You seem to have set the LD_LIBRARY_PATH variable, this might cause *'
  echo '* some trouble during the installation of this package.		      *'
  echo '* If install.sh does not finish successfully, clear		      *'
  echo '* LD_LIBRARY_PATH from the environment, and start over.		      *'
  echo '***************************** WARNING *********************************'
fi

TMPF=/tmp/list.id.$$

trap "/bin/rm -f $TMPF; exit 1" 1 2 3 13 15

/bin/rm -f $TMPF

echo Id test >$TMPF

AM_ROOT=no

if ls -l $TMPF | grep '^[^ ]*  *[0-9][0-9]*  *root ' >/dev/null
then
  /bin/rm -f $TMPF
  AM_ROOT=yes
  installerid=`ls -l ../Makefile |
   sed -e 's/^[^ ]* *[0-9][0-9]*[^0-9] *\([^ ]*\) .*$/\1/'`
  listid=`ls -ld $target/. |
   sed -e 's/^[^ ]* *[0-9][0-9]*[^0-9] *\([^ ]*\) .*$/\1/'`
  if test root = $listid
  then
     echo "Please give $target the right owner & group first"
     exit 2
  fi
else
  /bin/rm -f $TMPF
  ( exec 2>/dev/null; echo Id test >targetdir.tmp )
  if test $? != 0
  then	# You can run install.sh WITHOUT root permissions as well!
     echo "Please run install.sh with root permissions instead"
     exit 77
  fi
  echo 1>&2 \
   "*** This script is best run as root, if that's not possible press return"
  echo 1>&2 \
   "*** to continue; if it *is* possible, abort now and restart as root!"
  read a
  listid=`ls -l install.sh |
   sed -e 's/^[^ ]* *[0-9][0-9]*[^0-9] *\([^ ]*\) .*$/\1/'`
fi

trap "" 1 2 3 13 15

export listid

rm -f install.list
date >install.list
chmod 0666 install.list

if test $AM_ROOT = yes
then
  if test ! -f $setid
  then
     echo "Please execute the following commands first:"
     echo ""
     echo "	cd ..; make setid; cd SmartList"
     echo ""
     echo "Then run this script again."
     exit 64
  fi
  exec 4<&0
  case $installerid in
     [0-9]*) . ./install.sh2 ;;
     *) $setid $installerid <install.sh2 || exit 1;;
  esac
  $setid $listid $target <install.sh3 || exit 64
  exec 4<&-
  echo "Making $target/$bindir/flist suid root..."
  if chown root "$target/$bindir/flist" && chmod 04755 "$target/$bindir/flist"
  then
  :
  else
     echo "You either have to symlink the $target/$bindir"
     echo "directory to a partition where root has root permissions;"
     echo "or make sure that root can use its rights on the existing"
     echo "partition (that contains $target/$bindir)."
     echo "Then run this script again."
     exit 64
  fi
else
  exec 4<&0
  . ./install.sh2
  . ./install.sh3
  exec 4<&-
fi

chmod 0644 install.list

echo '**********************************************************************'
echo "Finished installing, now you should:"
echo ""
if test -f $target/.etc/rc.init.new
then
  echo "	edit $target/.etc/rc.init.new"
  echo ""
  echo "AND then (preserving hardlinks!):"
  echo ""
  echo "	cat $target/.etc/rc.init.new >$target/.etc/rc.init"
else
  echo "	edit $target/.etc/rc.init"
fi
echo ""
echo "so that \`PATH', \`domain' and \`listmaster' reflect your installation."
if test -f $target/.etc/rc.init.new
then
  echo "Finally, to reenable the lists execute:"
  echo ""
  echo "		/bin/rm -f $target/.etc/rc.lock"
  echo ""
  touch "$target/.etc/rc.lock"
fi
echo '**********************************************************************'
