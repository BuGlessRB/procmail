#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.10 1993/01/28 14:21:59 berg Exp $

test $# != 1 && echo "Usage: install.sh target-directory" && exit 1

target="$1"

test ! -d "$target" && echo "Please create the target directory first" &&
 exit 2

FRAGILE="rc.init "
DIRS="bin etc"

echo "Preserving any old files: $FRAGILE"

for a in $FRAGILE
do
  test -f "$target/.etc/$a" &&
   mv -f "$target/.etc/$a" "$target/.etc/$a.old"
done

echo Installing...

for a in $DIRS
do
  mkdir "$target/.$a" 2>/dev/null
  cp $a/* "$target/.$a"
done

chmod 0640 "$target/.etc/rc.custom" "$target/.etc/rc.init"

for a in $FRAGILE
do
  if test -f "$target/.etc/$a.old"
  then
     mv -f "$target/.etc/$a" "$target/.etc/$a.new"
     mv -f "$target/.etc/$a.old" "$target/.etc/$a"
  fi
done

cp Manual "$target/.etc"
mv -f "$target/.bin/procmail" "$target/.bin/.procmail" 2>/dev/null
chmod 0755 $target/.bin/*
mv -f "$target/.bin/.procmail" "$target/.bin/procmail" 2>/dev/null

for a in $DIRS
do
  ls -ld "$target/.$a" $target/.$a/*
done

cd ../src
test -f multigram || make multigram
cp multigram "$target/.bin"
cd ../mailinglist

ln -f "$target/.bin/multigram" "$target/.bin/idhash" 2>/dev/null
ln -f "$target/.bin/multigram" "$target/.bin/flist" 2>/dev/null
chmod 04755 "$target/.bin/flist"
ls -l "$target/.bin/multigram" "$target/.bin/idhash" "$target/.bin/flist"

echo Creating link from .etc/rc.main to .procmailrc
rm -f "$target/.procmailrc"
ln "$target/.etc/rc.main" "$target/.procmailrc"

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $target/.etc/rc.init to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
