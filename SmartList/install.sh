#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.4 1992/12/01 15:31:41 berg Exp $

test $# != 1 && echo "Usage: install.sh target-directory" && exit 1

target="$1"

test ! -d "$target" && echo "Please create the target directory first" &&
 exit 2

FRAGILE="listrc.main the_lists"
DIRS="bin listadmin"

echo "Preserving any old files: $FRAGILE"

for a in $FRAGILE
do
  mv -f "$target/.listadmin/$a" "$target/.listadmin/$a.old"
done

echo Installing...

for a in $DIRS
do
  mkdir "$target/.$a" 2>/dev/null
  cp $a/* "$target/.$a"
done

for a in $FRAGILE
do
  if test -f "$target/.listadmin/$a.old"
  then
     mv -f "$target/.listadmin/$a" "$target/.listadmin/$a.new"
     mv -f "$target/.listadmin/$a.old" "$target/.listadmin/$a"
  fi
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
ls -l "$target/.bin/multigram"

echo Creating link from .listadmin/listrc.main to .procmailrc
rm -f "$target/.procmailrc"
ln "$target/.listadmin/listrc.main" "$target/.procmailrc"

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $a/.listadmin/listrc.main to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
