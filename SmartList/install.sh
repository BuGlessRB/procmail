#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.6 1993/01/13 16:17:06 berg Exp $

test $# != 1 && echo "Usage: install.sh target-directory" && exit 1

target="$1"

test ! -d "$target" && echo "Please create the target directory first" &&
 exit 2

DIRS="bin etc"

echo Installing...

for a in $DIRS
do
  mkdir "$target/.$a" 2>/dev/null
  cp $a/* "$target/.$a"
done

chmod 755 $target/.bin/*

for a in $DIRS
do
  ls -ld "$target/.$a" $target/.$a/*
done

cd ../src
test -f multigram || make multigram
cp multigram "$target/.bin"
cd ../mailinglist

ln "$target/.bin/multigram" "$target/.bin/idhash"
ln "$target/.bin/multigram" "$target/.bin/flist"
chmod 4755 "$target/.bin/flist"
ls -l "$target/.bin/multigram" "$target/.bin/idhash" "$target/.bin/flist"

echo Creating link from .etc/rc.main to .procmailrc
rm -f "$target/.procmailrc"
ln "$target/.etc/rc.main" "$target/.procmailrc"

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $a/.etc/rc.init to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
