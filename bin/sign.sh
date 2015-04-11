#!/bin/bash -x

#!/bin/bash -x
DIR="$(dirname "$0")/../"

timestamp=0
clean=0;

. "$DIR"/etc/sign.conf
[ -e "$DIR"/etc/local/sign.conf ] && . "$DIR"/etc/local/sign.conf

for d in "$DIR"/src/{Release,Debug}/Setup\ Files
do
	# quick test to see if there is anything to do
	[ -e "$d/CipherShed.exe" ] || continue
	pushd "$d"
	BUILDINSTALLER=0
	# are we "resetting" the files?
	if [ $clean == 1 ]; 
	then
		[ -e CipherShed.exe.presignbak ]         && rm CipherShed.exe                && mv CipherShed.exe.presignbak         CipherShed.exe
		[ -e CipherShed\ Format.exe.presignbak ] && rm CipherShed\ Format.exe        && mv CipherShed\ Format.exe.presignbak CipherShed\ Format.exe 
		[ -e truecrypt.sys.presignbak ]          && rm truecrypt.sys                 && mv truecrypt.sys.presignbak          truecrypt.sys
		[ -e truecrypt-x64.sys.presignbak ]      && rm truecrypt-x64.sys             && mv truecrypt-x64.sys.presignbak      truecrypt-x64.sys 
		BUILDINSTALLER=1
	fi
	# sign all the non-driver files
	for i in CipherShed.exe CipherShed\ Format.exe
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && "$SIGNTOOL" sign /v /n "$CRT" "$i" || exit $?
			BUILDINSTALLER=1
		fi
	done
	# sign all the driver files
	for i in *.sys
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && "$SIGNTOOL" sign /v /ac "$MSXCRT" /n "$CRT" "$i" || exit $?
			BUILDINSTALLER=1
		fi
	done
	if [ BUILDINSTALLER == 1 -o ! -f CipherShed-Setup-0.7.3.exe ]
	then
		# remove the installer files first
		for i in CipherShed-Setup-0.7.3.exe*
		do
			rm -f "$i"
		done
		# build the installer
		./CipherShed-Setup.exe /pq
	fi
	for i in CipherShed-Setup-0.7.3.exe
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && "$SIGNTOOL" sign /v /n "$CRT" "$i"
		fi
	done
	popd
done
