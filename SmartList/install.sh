#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.14 1993/04/02 12:38:18 berg Exp $

test $# != 1 && echo "Usage: install.sh target-directory [.bin]" && exit 1

target="$1"

test ! -d "$target" && echo "Please create the target directory first" &&
 echo "Make sure that the target directory has the right owner" && exit 2

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

cd ../src
test -f multigram || make multigram
cp multigram "$target/.bin"
cd ../mailinglist

cp Manual "$target/.etc"
mv -f "$target/.bin/procmail" "$target/.bin/.procmail" 2>/dev/null
chmod 0755 $target/.bin/*
for a in flist senddigest idhash
do
  ln -f "$target/.bin/multigram" "$target/.bin/$a" 2>/dev/null
done
chmod 04755 "$target/.bin/flist"
mv -f "$target/.bin/.procmail" "$target/.bin/procmail" 2>/dev/null

for a in $DIRS
do
  ls -ld "$target/.$a" $target/.$a/*
done

echo Creating link from .etc/rc.main to .procmailrc
rm -f "$target/.procmailrc"
ln "$target/.etc/rc.main" "$target/.procmailrc"

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $target/.etc/rc.init to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
