#! /bin/sh
: &&O= || exec /bin/sh $0 $argv:q # we're in a csh, feed myself to sh
#$Id: install.sh,v 1.3 1992/11/24 16:57:59 berg Exp $

test $# != 1 && echo "Usage: install.sh target-directory" && exit 1

target="$1"

test ! -d "$target" && echo "Please create the target directory first" &&
 exit 2

echo Installing...

for a in bin listadmin
do
  mkdir "$target/.$a"
  cp $a/* "$target/.$a"
  test $a = bin && chmod 755 $target/.$a/*
  ls -ld "$target/.$a" $target/.$a/*
done

cd ../src
test -f multigram || make multigram
cp multigram "$target/.bin"
cd ../mailinglist
ls -l "$target/.bin/multigram"

echo Creating link from .listadmin/listrc.main to .procmailrc
ln "$target/.listadmin/listrc.main" "$target/.procmailrc"

echo '**********************************************************************'
echo "Finished installing, now you should"
echo "edit $a/.listadmin/listrc.main to make sure"
echo "that \`PATH', \`domain' and \`listmaster' reflect your installation."
echo '**********************************************************************'
