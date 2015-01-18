#!/bin/bash

unset _MSVC16_ROOT
unset MSVC16_ROOT
unset _PKCS11_INC
unset PKCS11_INC
unset _WINDDK_ROOT
unset WINDDK_ROOT

# allow local overrides...
[ -e "$(dirname "$0")/etc/local/env.sh" ] && . "$(dirname "$0")/etc/local/env.sh"

#_MSVC16_ROOT="$(dirname "$0")/../var/opt/microsoft-visual-c-152c/files/iso/MSVC15/"
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #crash
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #weird hang?
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #weird hang?
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #weird hang?
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #Out of memory - cl.exe : return code '0x2'
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #cl.exe : return code '0x2'
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #cl.exe : return code '0x2'
#_MSVC16_ROOT="/cygdrive/c/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/MSVC15/" #longest working path

#default location is c:\msvc15
_MSVC16_ROOT="${_MSVC16_ROOT-"/cygdrive/c/MSVC15/"}"
MSVC16_ROOT="${MSVC16_ROOT-"$(cygpath -wa "$_MSVC16_ROOT" )"}"

#default location is src/Pkcs11
_PKCS11_INC="${_PKCS11_INC-"$(dirname "$0")/../src/Pkcs11/"}"
PKCS11_INC="${PKCS11_INC-"$(cygpath -wa "$_PKCS11_INC" )"}"

#default location is c:\WinDDK\7600.16385.1
_WINDDK_ROOT="${_WINDDK_ROOT-"/cygdrive/c/WinDDK/7600.16385.1/"}"
WINDDK_ROOT="${WINDDK_ROOT-"$(cygpath -wa "$_WINDDK_ROOT" )"}"

PATH="$( realpath "$_MSVC16_ROOT/bin/" )":"$(realpath "$(dirname "$0")/../var/opt/nasm-2.08/")":$PATH

export MSVC16_ROOT
export PKCS11_INC
export WINDDK_ROOT
export PATH
