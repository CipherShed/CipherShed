#!/bin/bash

FILE=TrueCrypt\ 6.3\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.3\ Source.tar.gz'' -C .'
DATE='Wed, 21 Oct 2009 13:47:36 +0200'
VER=6.3
FIXPATHCMD='rsync -a truecrypt-6.3-source/ src; rm -rf truecrypt-6.3-source/'
VERS=true
VERE=false
STEP=38;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

