#!/bin/bash
#
#init
#
#
mkdir -p tmp
grep ^tmp/$ .gitignore > /dev/null  || (echo tmp/ >> .gitignore)
nano tmp/readme-history.txt
cp tmp/readme-history.txt .
git add readme-history.txt
git add bin/ etc/
git add .gitignore
git commit "$@" -m 'Initial workspace for history extraction'
