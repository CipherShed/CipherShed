#!/bin/bash

FILE=TrueCrypt\ 6.2a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.2a\ Source.tar.gz'' -C .'
DATE='Mon, 15 Jun 2009 10:15:38 +0200'
VER=6.2a
FIXPATHCMD='rsync -a truecrypt-6.2a-source/ src; rm -rf truecrypt-6.2a-source/'
VERS=true
VERE=false
STEP=36;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

