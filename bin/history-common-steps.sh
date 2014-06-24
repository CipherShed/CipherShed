#!/bin/bash

STEPCV=16

echo $STEP,$STEPFV,$STEPCV '->' $FILE

[ "$VERS" == true ] && git rm -q files/*
mkdir -p files/

cp tmp/"$FILE"* files/
git add files/*

# rm -rf src/

[ -f $(dirname $0)/../etc/history-step-$STEP.srcmod ] && . $(dirname $0)/../etc/history-step-$STEP.srcmod

mkdir -p src/
eval $EXTCMD
eval $FIXPATHCMD

[ -f $(dirname $0)/../etc/history-step-$STEP.postsrcmod ] && . $(dirname $0)/../etc/history-step-$STEP.postsrcmod

git status | sed '1,/^# *Changes not staged for commit:/d' | grep -E '^#\s+deleted:' | cut -c 15- | while read line; do git rm "$line"; done

find src/ -type f > tmp/files.src

grep -Fvf tmp/files.exempt  tmp/files.src > tmp/files.todos2unix

grep -Ff tmp/files.conversion-unix2dos-utf  tmp/files.src > tmp/files.tounix2dos-utf

grep -Ff tmp/files.conversion-unix2dos  tmp/files.src > tmp/files.tounix2dos

grep -Ff tmp/files.conversion-dos2unix-utf  tmp/files.src > tmp/files.todos2unix-utf

cat tmp/files.todos2unix | while read line; do dos2unix -q -k "$line"; done

cat tmp/files.tounix2dos-utf | while read line; do unix2dos -q -m -k "$line"; done

cat tmp/files.tounix2dos | while read line; do unix2dos -q -k "$line"; done

cat tmp/files.todos2unix-utf | while read line; do dos2unix -q -m -k "$line"; done

git add src/

git diff -w --cached src/ > tmp/patch-noeol.diff

git status > tmp/git-status.log

cat tmp/patch-noeol.diff | grep ^diff | perl -pe 's|^.*?/||; s|b/.*?$||; s/ $//;'| sort > tmp/files.legit

cat tmp/patch-noeol.diff | grep ^Binary | perl -ne 's|^Binary files ||; s|^.* and b/(.*) differ$|\1|; print unless m|^.* and /dev/null differ$|' > tmp/files.bin

cat tmp/git-status.log | grep -E '^#\s+modified:'| cut -c 15- | sort > tmp/files.modified

grep -Fvf tmp/files.legit  tmp/files.modified > tmp/files.toreset

#grep -Fvf tmp/files.bin  tmp/files.legit > tmp/files.tocheckeol

#cat tmp/files.tocheckeol | while read line ; do echo $line; Lw=$(git diff -w --cached "$line"|wc -l ); L=$(git diff --cached "$line"|wc -l ); [ "$Lw" != "$L" ] && (D=$(date -r "$line"); dos2unix "$line"; touch -d "$D" "$line"); done

# [[ "$AUTOFIXEOL" == true ]] && cat tmp/files.toreset | while read line; do git reset "$line"; git checkout -- "$line"; done

if [ -f files/"$FILE".sig ]; then

	gpg --verify files/"$FILE".sig 2> tmp/gpg-verify-log.txt
	GIT_AUTHOR="$(cat tmp/gpg-verify-log.txt | grep @ | sed 's/^gpg: Good signature from "//; s/"$//;')"
	echo gpg verify $FILE.sig > tmp/gpg-commit-msg.txt
	cat tmp/gpg-verify-log.txt >> tmp/gpg-commit-msg.txt

else
	GIT_AUTHOR="Uknown Anonymous Author <unknown@truecrypt-foundation.org>"
	echo missing "$FILE".sig, cannot gpg verify source $FILE > tmp/gpg-commit-msg.txt
fi;

echo -e "TrueCrypt v$VER - $FILE" > tmp/commit-log.txt
echo >> tmp/commit-log.txt
echo extracted $file to src/ >> tmp/commit-log.txt
echo >> tmp/commit-log.txt

cat tmp/gpg-commit-msg.txt >> tmp/commit-log.txt

echo >> tmp/commit-log.txt

echo import script note: step $STEP formula $STEPFV common $STEPCV >> tmp/commit-log.txt

GIT_AUTHOR_DATE="$DATE"
[[ "x$1" == "x" ]] && git commit -q -F tmp/commit-log.txt --author="$GIT_AUTHOR" --date="$GIT_AUTHOR_DATE"
