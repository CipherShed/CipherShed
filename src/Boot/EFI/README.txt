CipherShed EFI boot loader
--------------------------

- The source code is based on the GNU EFI package, see http://sourceforge.net/projects/gnu-efi/
- The development was done at a debian system with version 3.0.2-1 of GNU EFI.
- GNU EFI does not support all required EFI protocol interfaces. Hence, some header files are taken
  from the EDK2 project. These files are located in the "include/edk2" directory.
- To build the CipherShed EFI loader, type "make" in the "Boot/EFI" directory.
- To build the CipherShed EFI loader for a dedicated architecture, type "ARCH=<arch> make" 
  in the "Boot/EFI" directory, where <arch> must be either ia32 or x86_64.
- If the ARCH variable is not set, the destination architecture is detected by the make process, 
  see Makefile.inc. Only "ia32" and "x86_64" are supported.
- To install the loader, see the paper "../../../doc/devdocs/boot/efi/cs_efi_loader.odt".
