#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.23 1993/06/28 17:02:37 berg Exp $

SHELL=/bin/shell
export SHELL

test $# != 1 -a $# != 2 && echo "Usage: install.sh target-directory [.bin]" &&
 exit 64

target="$1"
bindir="$2"

test -z "$bindir" && bindir=.bin

test ! -d "$target" && echo "Please create the target directory first" &&
 exit 2

if binmail=`procmail /dev/null DEFAULT=/dev/null LOG=\\\$SENDMAIL \
  </dev/null 2>&1`
then
:
else
  echo "Please make sure that procmail is on our PATH"
  exit 64
fi

if expr "X$bindir" : X.bin >/dev/null
then
:
else
  echo "I prefer a bin directory that starts with .bin"
  echo "If you want to enforce a different name, patch install.sh first :-)."
  exit 64
fi

cd "`dirname $0`"
PATH=.:$PATH

export target bindir binmail PATH

TMPF=/tmp/list.id.$$

trap "/bin/rm -f $TMPF; exit 1" 1 2 3 15

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
  if ( echo Id test >id.test ) 2>/dev/null
  then
  :
  else
     echo "Please run install.sh with root permissions instead"
     exit 77
  fi
  /bin/rm -f id.test
  listid=`ls -l install.sh |
   sed -e 's/^[^ ]* *[0-9][0-9]*[^0-9] *\([^ ]*\) .*$/\1/'`
fi

trap "" 1 2 3 15

export listid

if test $AM_ROOT = yes
then
  case $installerid in
     [0-9]*) . ./install.sh2;;
     *) su $installerid ./install.sh2;;
  esac
  su $listid ./install.sh3
  echo "Making $target/$bindir/flist suid root..."
  chown root "$target/$bindir/flist"
  chmod 04755 "$target/$bindir/flist"
else
  . ./install.sh2
  . ./install.sh3
fi

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $target/.etc/rc.init to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
