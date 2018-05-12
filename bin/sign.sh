#!/bin/bash

DIR="$(dirname "$0")/../"
DIR="$(cd "$DIR"; pwd)"

[[ $(uname -s) == CYGWIN* ]] && CYGWIN=1 || CYGWIN=0

[ $CYGWIN ] && PCV="cygpath -w" || PCV="echo"

exitmsg()
{
 code=$1
 shift
 1>&2 echo "$*"
 exit $code
}

warn()
{
 1>&2 echo "$*"
}

[ "x$1" == "x" ] && p=sha1,sha256 || p=$1

modes=()

while IFS=',' read -ra ADDR; do
	for i in "${ADDR[@]}"; do
		modes+=("$i")
		# process "$i"
	done
done <<< "$p"

if [ ! ${#modes[@]} -gt 0 ]; then
	exitmsg 1 'no mode(s) set'
fi

VERSION=$(echo '_CS_VERSION_DOTTED_5STR' | cpp -P -x c -include "$DIR/src/include/version.h")

. "$DIR"/etc/sign.conf
[ -e "$DIR"/etc/local/sign.conf ] && . "$DIR"/etc/local/sign.conf

crts=()
xcrts=()
for mode in "${modes[@]}"; do
	case "$mode" in
		*\ * )
		exitmsg 1 "invalid mode: $mode"
		;;
	esac

	#######################################
	# CRTs
	#######################################

	iCRT=CRT$mode
	eval mCRT=\"\$$iCRT\"
	if [ "x${mCRT}" == "x" ]; then
		if [ "x${CRT}" == "x" ]; then
			exitmsg 1 "blank CRT entry and no default for mode $mode"
		else
			warn "WARNING: for mode: $mode using default CRT: $CRT"
			mCRT="$CRT"
		fi
	fi

	if [ ! "${mCRT:0:1}" = "/" ]; then
		mCRT="$DIR"/"$mCRT"
	fi

	if [ ! -f "$mCRT" ]; then
		exitmsg 1 "CRT file not found: $mCRT"
	fi

	crts+=( "$($PCV "$mCRT")" )

	#######################################
	# X CRTs
	#######################################

	iCRT=XCRT$mode
	eval mCRT=\"\$$iCRT\"
	if [ "x${mCRT}" == "x" ]; then
		if [ "x${XCRT}" == "x" ]; then
			exitmsg 1 "blank X-CRT entry and no default for mode $mode"
		else
			warn "WARNING: for mode: $mode using default X-CRT: $XCRT"
			mCRT="$XCRT"
		fi
	fi

	if [ ! "${mCRT:0:1}" = "/" ]; then
		mCRT="$DIR"/"$mCRT"
	fi

	if [ ! -f "$mCRT" ]; then
		exitmsg 1 "X-CRT file not found: $mCRT"
	fi

	xcrts+=( "$($PCV "$mCRT")" )

done

if [ ! ${#crts[@]} -gt 0 ]; then
	exitmsg 1 'no CRT(s) set'
fi

if [ ! ${#xcrts[@]} -gt 0 ]; then
	exitmsg 1 'no X-CRT(s) set'
fi



timestamp=0
clean=0;



sign()
{
 trap 'previous_command=$this_command; this_command=$BASH_COMMAND' DEBUG
 for ((i=0;i<${#crts[@]};++i)); do
	"$SIGNTOOL" sign /fd "${modes[i]}" /v /f "${crts[i]}" /as $TIMESTAMPOPT $TIMESTAMPURL "$*" || exitmsg $? failed: $previous_command $(eval "echo as: $previous_command")
 done
 trap - DEBUG
}


xsign()
{
 trap 'previous_command=$this_command; this_command=$BASH_COMMAND' DEBUG
 for ((i=0;i<${#crts[@]};++i)); do
	"$SIGNTOOL" sign /fd "${modes[i]}" /ph /v /ac "${xcrts[i]}" /f "${crts[i]}" /as $TIMESTAMPOPT $TIMESTAMPURL "$*" || exitmsg $? failed: $previous_command $(eval "echo as: $previous_command")
 done
 trap - DEBUG
}



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
		[ -e CipherShed-Format.exe.presignbak ]  && rm CipherShed-Format.exe         && mv CipherShed-Format.exe.presignbak CipherShed-Format.exe 
		[ -e truecrypt.sys.presignbak ]          && rm truecrypt.sys                 && mv truecrypt.sys.presignbak          truecrypt.sys
		[ -e truecrypt-x64.sys.presignbak ]      && rm truecrypt-x64.sys             && mv truecrypt-x64.sys.presignbak      truecrypt-x64.sys 
		BUILDINSTALLER=1
	fi
	# sign all the non-driver files
	for i in CipherShed.exe CipherShed-Format.exe
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && sign "$i" || exit $?
			BUILDINSTALLER=1
		fi
	done
	# sign all the driver files
	for i in *.sys
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && xsign "$i" || exit $?
			BUILDINSTALLER=1
		fi
	done
	if [ BUILDINSTALLER == 1 -o ! -f CipherShed-Setup-$VERSION.exe ]
	then
		# remove the installer files first
		for i in CipherShed-Setup-$VERSION.exe*
		do
			rm -f "$i"
		done
		# build the installer
		./CipherShed-Setup.exe /pq
	fi
	for i in CipherShed-Setup-$VERSION.exe
	do
		if [ ! -e "$i".presignbak ];
		then
			cp "$i" "$i".presignbak && sign "$i" || exit $?
		fi
	done
	popd
done
