This archive contains the source code of CipherShed.

Important
=========
You may use the source code contained in this archive only if you accept and
agree to the license terms contained in the file 'License.txt', which is
included in this archive.

Additional information, including building on Windows, is available on our
website, https://ciphershed.org

The information on the website should be considered more up-to-date
and supersedes that of this document.

Linux and Mac OS X
======================
Requirements for Building CipherShed for Linux and Mac OS X:
-----------------------------------------------------------
- GNU Make
- GNU C++ Compiler 4.0 or compatible
- Apple Xcode (Mac OS X only)
- NASM assembler 2.08 or compatible (x86/x64 architecture only)
- pkg-config
- wxWidgets 2.8/3.0.1 shared library and header files installed or
  wxWidgets 2.8/3.0.1 library source code (available at http://www.wxwidgets.org)
- FUSE library and header files (available at http://fuse.sourceforge.net
  and http://code.google.com/p/macfuse)
- RSA Security Inc. PKCS #11 Cryptographic Token Interface (Cryptoki) 2.20
  header files (available at ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20)
  located in a standard include path or in a directory defined by the
  environment variable 'PKCS11_INC'.

Instructions for Building CipherShed for Linux and Mac OS X:
-----------------------------------------------------------
1) Change the current directory to the root of the CipherShed source code.

2) If you have no wxWidgets shared library installed, run the following
   command to configure the wxWidgets static library for CipherShed and to
   build it: 

   $ make WX_ROOT=/usr/src/wxWidgets wxbuild

   The variable WX_ROOT must point to the location of the source code of the
   wxWidgets library. Output files will be placed in the './wxrelease/'
   directory.

3) To build CipherShed, run the following command:

   $ make

   or if you have no wxWidgets shared library installed:
   
   $ make WXSTATIC=1

4) If successful, the CipherShed executable should be located in the directory
   'Main'.

By default, a universal executable supporting both graphical and text user
interface is built. To build a console-only executable, which requires no GUI
library, use the 'NOGUI' parameter:

   $ make NOGUI=1 WX_ROOT=/usr/src/wxWidgets wxbuild
   $ make NOGUI=1 WXSTATIC=1

Notes
-----------------------------------------------------------

- If you run into numerous compilation errors while building CipherShed, double
  check to make sure that you installed an official release of wxwidgets and
  not an unofficial build. 

Legal Information
====================
Copyright Information
---------------------
This software as a whole:
Copyright (c) 2014 The CipherShed Project. All rights reserved.
Copyright (c) 2012 TrueCrypt Developers Association. All rights reserved.

Portions of this software:
Copyright (c) 2003-2012 TrueCrypt Developers Association. All rights reserved.
Copyright (c) 1998-2000 Paul Le Roux. All rights reserved.
Copyright (c) 1998-2008 Brian Gladman, Worcester, UK. All rights reserved.
Copyright (c) 2002-2004 Mark Adler. All rights reserved.
For more information, please see the legal notices attached to parts of the
source code.

Based on TrueCrypt, freely available at http://www.truecrypt.org/

Trademark Information
---------------------

Any trademarks contained in the source code, binaries, and/or in the 
documentation, are the sole property of their respective owners.
