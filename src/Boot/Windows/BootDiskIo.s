	.arch i8086,jumps
	.code16
	.att_syntax prefix
#NO_APP
	.global	SectorBuffer
	.bss
	.p2align	0
	.type	SectorBuffer, @object
	.size	SectorBuffer, 512
SectorBuffer:
	.skip	512,0
	.data
	.type	_ZZ14IsLbaSupportedhE11CachedDrive, @object
	.size	_ZZ14IsLbaSupportedhE11CachedDrive, 1
_ZZ14IsLbaSupportedhE11CachedDrive:
	.byte	127
	.local	_ZZ14IsLbaSupportedhE12CachedStatus
	.comm	_ZZ14IsLbaSupportedhE12CachedStatus,1,1
	.text
	.global	_Z14IsLbaSupportedh
	.type	_Z14IsLbaSupportedh, @function
_Z14IsLbaSupportedh:
	pushw	%bp
	movw	%sp,	%bp
	subw	$2,	%sp
	movw	$0,	-2(%bp)
	movb	_ZZ14IsLbaSupportedhE11CachedDrive,	%al
	cmpb	4(%bp),	%al
	cmpb	4(%bp),	%al
	je	.L5
#APP
;# 76 "BootDiskIo.cpp" 1
	
		mov $0x55aa, %bx
		mov 4(%bp), %dl
		mov $0x41, %ah
		int $0x13
		jc err
		mov %bx, %ax
err:

;# 0 "" 2
#NO_APP
	movw	%ax,	-2(%bp)
	movb	4(%bp),	%al
	movb	%al,	_ZZ14IsLbaSupportedhE11CachedDrive
	cmpw	$-21931,	-2(%bp)
	lahf
	shlb	%ah
	cwtd
	movb	%dl,	%al
	movb	%al,	%al
	movb	$0,	%ah
	subb	%al,	%ah
	movb	%ah,	%al
	movb	%al,	_ZZ14IsLbaSupportedhE12CachedStatus
	jmp	.L3
.L5:
	nop
.L3:
	movb	_ZZ14IsLbaSupportedhE12CachedStatus,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z14IsLbaSupportedh, .-_Z14IsLbaSupportedh
	.section	.rodata
.LC0:
	.string	"Write"
.LC1:
	.string	"Read"
.LC2:
	.string	" error:"
.LC3:
	.string	" Drive:"
.LC4:
	.string	" Sector:"
.LC5:
	.string	" CHS:"
	.text
	.global	_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress
	.type	_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress, @function
_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress:
	pushw	%bp
	movw	%sp,	%bp
	subw	$8,	%sp
	call	_Z9PrintEndlv
	cmpb	$1,	6(%bp)
	cmpb	$0,	6(%bp)
	je	.L7
	movw	$.LC0,	%ax
	jmp	.L8
.L7:
	movw	$.LC1,	%ax
.L8:
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	movw	$.LC2,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	movb	4(%bp),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-4(%bp)
	movw	$0,	-2(%bp)
	pushw	-2(%bp)
	pushw	-4(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	movw	$.LC3,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	movb	8(%bp),	%al
	xorb	$-128,	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-8(%bp)
	movw	$0,	-6(%bp)
	pushw	-6(%bp)
	pushw	-8(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	cmpw	$1,	10(%bp)
	cmpw	$0,	10(%bp)
	je	.L9
	movw	$.LC4,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	pushw	10(%bp)
	call	_Z5PrintRK13UINT64_STRUCT
	addw	$2,	%sp
.L9:
	cmpw	$1,	12(%bp)
	cmpw	$0,	12(%bp)
	je	.L10
	movw	$.LC5,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	pushw	12(%bp)
	call	_Z5PrintRK10ChsAddress
	addw	$2,	%sp
.L10:
	call	_Z9PrintEndlv
	call	_Z4Beepv
	nop
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress, .-_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress
	.global	_Z5PrintRK10ChsAddress
	.type	_Z5PrintRK10ChsAddress, @function
_Z5PrintRK10ChsAddress:
	pushw	%bp
	movw	%sp,	%bp
	subw	$12,	%sp
	movw	4(%bp),	%bx
	movw	(%bx),	%ax
	movw	%ax,	-4(%bp)
	movw	$0,	-2(%bp)
	pushw	-2(%bp)
	pushw	-4(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	movb	$47,	%al
	pushw	%ax
	call	_Z9PrintCharc
	addw	$2,	%sp
	movw	4(%bp),	%bx
	movb	2(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-8(%bp)
	movw	$0,	-6(%bp)
	pushw	-6(%bp)
	pushw	-8(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	movb	$47,	%al
	pushw	%ax
	call	_Z9PrintCharc
	addw	$2,	%sp
	movw	4(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-12(%bp)
	movw	$0,	-10(%bp)
	pushw	-10(%bp)
	pushw	-12(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	nop
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z5PrintRK10ChsAddress, .-_Z5PrintRK10ChsAddress
	.section	.rodata
.LC6:
	.string	" MB "
	.text
	.global	_Z20PrintSectorCountInMBRK13UINT64_STRUCT
	.type	_Z20PrintSectorCountInMBRK13UINT64_STRUCT, @function
_Z20PrintSectorCountInMBRK13UINT64_STRUCT:
	pushw	%bp
	movw	%sp,	%bp
	subw	$8,	%sp
	leaw	-8(%bp),	%dx
	movw	$11,	%ax
	pushw	%ax
	pushw	4(%bp)
	pushw	%dx
	call	_ZrsRK13UINT64_STRUCTi
	addw	$6,	%sp
	leaw	-8(%bp),	%ax
	pushw	%ax
	call	_Z5PrintRK13UINT64_STRUCT
	addw	$2,	%sp
	movw	$.LC6,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	nop
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z20PrintSectorCountInMBRK13UINT64_STRUCT, .-_Z20PrintSectorCountInMBRK13UINT64_STRUCT
	.global	_Z16ReadWriteSectorsbjjhRK10ChsAddresshb
	.type	_Z16ReadWriteSectorsbjjhRK10ChsAddresshb, @function
_Z16ReadWriteSectorsbjjhRK10ChsAddresshb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$6,	%sp
	movw	12(%bp),	%bx
	movw	(%bx),	%ax
	movb	%al,	-3(%bp)
	movw	12(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	-4(%bp)
	movw	12(%bp),	%bx
	movw	(%bx),	%ax
	movb	$2,	%dl
	movb	%dl,	%cl
	shrw	%cl,	%ax
	movb	%al,	%al
	movb	%al,	%al
	andb	$-64,	%al
	movb	%al,	%ah
	movb	-4(%bp),	%al
	orb	%ah,	%al
	movb	%al,	-4(%bp)
	cmpb	$1,	4(%bp)
	cmpb	$0,	4(%bp)
	je	.L14
	movb	$3,	%al
	jmp	.L15
.L14:
	movb	$2,	%al
.L15:
	movb	%al,	-5(%bp)
	movb	$5,	-2(%bp)
.L20:
	movb	$0,	-1(%bp)
	movw	12(%bp),	%bx
	movw	(%bx),	%ax
	movw	2(%bx),	%dx
#APP
;# 194 "BootDiskIo.cpp" 1
	
			push %es
			mov %ax, 6(%bp)
			mov	%ax, %es
			mov	8(%bp), %bx
			mov 10(%bp), %dl
			mov -3(%bp), %ch
			mov %ax, %si
			mov 2(%si), %dh
			mov -4(%bp), %cl
			mov	14(%bp), %al
			mov	-5(%bp), %ah
			int	$0x13
			jnc ok				// If CF=0, ignore AH to prevent issues caused by potential bugs in BIOSes
			mov	%ah, %al
ok:
			pop %es

;# 0 "" 2
#NO_APP
	movb	%al,	-1(%bp)
	cmpb	$17,	-1(%bp)
	cmpb	$17,	-1(%bp)
	jne	.L16
	movb	$0,	-1(%bp)
.L16:
	cmpb	$1,	-1(%bp)
	cmpb	$0,	-1(%bp)
	je	.L17
	movb	-2(%bp),	%al
	decb	%al
	movb	%al,	-2(%bp)
	cmpb	$1,	-2(%bp)
	cmpb	$0,	-2(%bp)
	je	.L17
	movb	$1,	%al
	jmp	.L18
.L17:
	movb	$0,	%al
.L18:
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L19
	jmp	.L20
.L19:
	movb	16(%bp),	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L21
	cmpb	$1,	-1(%bp)
	cmpb	$0,	-1(%bp)
	je	.L21
	pushw	12(%bp)
	movw	$0,	%ax
	pushw	%ax
	pushw	10(%bp)
	pushw	4(%bp)
	pushw	-1(%bp)
	call	_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress
	addw	$10,	%sp
.L21:
	movb	-1(%bp),	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z16ReadWriteSectorsbjjhRK10ChsAddresshb, .-_Z16ReadWriteSectorsbjjhRK10ChsAddresshb
	.global	_Z16ReadWriteSectorsbPhhRK10ChsAddresshb
	.type	_Z16ReadWriteSectorsbPhhRK10ChsAddresshb, @function
_Z16ReadWriteSectorsbPhhRK10ChsAddresshb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$2,	%sp
#APP
;# 223 "BootDiskIo.cpp" 1
	
        mov %cs, %ax

;# 0 "" 2
#NO_APP
	movw	%ax,	-2(%bp)
	movw	6(%bp),	%ax
	pushw	14(%bp)
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	%ax
	pushw	-2(%bp)
	pushw	4(%bp)
	call	_Z16ReadWriteSectorsbjjhRK10ChsAddresshb
	addw	$14,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z16ReadWriteSectorsbPhhRK10ChsAddresshb, .-_Z16ReadWriteSectorsbPhhRK10ChsAddresshb
	.global	_Z11ReadSectorsPhhRK10ChsAddresshb
	.type	_Z11ReadSectorsPhhRK10ChsAddresshb, @function
_Z11ReadSectorsPhhRK10ChsAddresshb:
	pushw	%bp
	movw	%sp,	%bp
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	6(%bp)
	pushw	4(%bp)
	movb	$0,	%al
	pushw	%ax
	call	_Z16ReadWriteSectorsbPhhRK10ChsAddresshb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z11ReadSectorsPhhRK10ChsAddresshb, .-_Z11ReadSectorsPhhRK10ChsAddresshb
	.global	_Z12WriteSectorsPhhRK10ChsAddresshb
	.type	_Z12WriteSectorsPhhRK10ChsAddresshb, @function
_Z12WriteSectorsPhhRK10ChsAddresshb:
	pushw	%bp
	movw	%sp,	%bp
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	6(%bp)
	pushw	4(%bp)
	movb	$1,	%al
	pushw	%ax
	call	_Z16ReadWriteSectorsbPhhRK10ChsAddresshb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z12WriteSectorsPhhRK10ChsAddresshb, .-_Z12WriteSectorsPhhRK10ChsAddresshb
	.type	_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb, @function
_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb:
	pushw	%di
	pushw	%bp
	movw	%sp,	%bp
	subw	$34,	%sp
	pushw	10(%bp)
	call	_Z14IsLbaSupportedh
	addw	$2,	%sp
	movb	%al,	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L30
	pushw	16(%bp)
	leaw	-24(%bp),	%ax
	pushw	%ax
	pushw	10(%bp)
	call	_Z16GetDriveGeometryhR13DriveGeometryb
	addw	$6,	%sp
	movb	%al,	%al
	movb	%al,	-3(%bp)
	cmpb	$1,	-3(%bp)
	cmpb	$0,	-3(%bp)
	je	.L31
	movb	-3(%bp),	%al
	jmp	.L33
.L31:
	leaw	-28(%bp),	%ax
	pushw	%ax
	pushw	12(%bp)
	leaw	-24(%bp),	%ax
	pushw	%ax
	call	_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress
	addw	$6,	%sp
	movw	14(%bp),	%ax
	movb	%al,	-33(%bp)
	movw	8(%bp),	%bx
	movw	4(%bx),	%ax
	movw	6(%bx),	%dx
	movw	%ax,	%di
	movw	8(%bp),	%bx
	movw	4(%bx),	%ax
	movw	6(%bx),	%dx
	movb	$0,	%cl
	movw	%dx,	%ax
	shrw	%cl,	%ax
	movw	%ax,	-32(%bp)
	movw	$0,	-30(%bp)
	movw	-32(%bp),	%ax
	pushw	16(%bp)
	pushw	-33(%bp)
	leaw	-28(%bp),	%dx
	pushw	%dx
	pushw	10(%bp)
	pushw	%di
	pushw	%ax
	pushw	6(%bp)
	call	_Z16ReadWriteSectorsbjjhRK10ChsAddresshb
	addw	$14,	%sp
	movb	%al,	%al
	jmp	.L33
.L30:
	movw	8(%bp),	%bx
	movb	$16,	(%bx)
	movw	8(%bp),	%bx
	movb	$0,	1(%bx)
	movw	8(%bp),	%bx
	movw	14(%bp),	%ax
	movw	%ax,	2(%bx)
	movw	8(%bp),	%bx
	movw	12(%bp),	%di
	movw	(%di),	%ax
	movw	%ax,	8(%bx)
	movw	2(%di),	%ax
	movw	%ax,	10(%bx)
	movw	4(%di),	%ax
	movw	%ax,	12(%bx)
	movw	6(%di),	%ax
	movw	%ax,	14(%bx)
	cmpb	$1,	6(%bp)
	cmpb	$0,	6(%bp)
	je	.L34
	movb	$67,	%al
	jmp	.L35
.L34:
	movb	$66,	%al
.L35:
	movb	%al,	-4(%bp)
	movb	$5,	-2(%bp)
.L40:
	movb	$0,	-1(%bp)
	movw	8(%bp),	%bx
	movw	(%bx),	%ax
	movw	%ax,	-20(%bp)
	movw	2(%bx),	%ax
	movw	%ax,	-18(%bp)
	movw	4(%bx),	%ax
	movw	%ax,	-16(%bp)
	movw	6(%bx),	%ax
	movw	%ax,	-14(%bp)
	movw	8(%bx),	%ax
	movw	%ax,	-12(%bp)
	movw	10(%bx),	%ax
	movw	%ax,	-10(%bp)
	movw	12(%bx),	%ax
	movw	%ax,	-8(%bp)
	movw	14(%bx),	%ax
	movw	%ax,	-6(%bp)
#APP
;# 303 "BootDiskIo.cpp" 1
	
			mov	$0x55aa, %bx
			mov	10(%bp), %dl
			mov -20(%bp), %si
			mov	-4(%bp), %ah
			xor %al, %al
			int	$0x13
			jnc ReadWriteSectors_ok				// If CF=0, ignore AH to prevent issues caused by potential bugs in BIOSes
			mov	%ah, %al
ReadWriteSectors_ok:

;# 0 "" 2
#NO_APP
	movb	%al,	-1(%bp)
	cmpb	$17,	-1(%bp)
	cmpb	$17,	-1(%bp)
	jne	.L36
	movb	$0,	-1(%bp)
.L36:
	cmpb	$1,	-1(%bp)
	cmpb	$0,	-1(%bp)
	je	.L37
	movb	-2(%bp),	%al
	decb	%al
	movb	%al,	-2(%bp)
	cmpb	$1,	-2(%bp)
	cmpb	$0,	-2(%bp)
	je	.L37
	movb	$1,	%al
	jmp	.L38
.L37:
	movb	$0,	%al
.L38:
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L39
	jmp	.L40
.L39:
	movb	16(%bp),	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L41
	cmpb	$1,	-1(%bp)
	cmpb	$0,	-1(%bp)
	je	.L41
	movw	$0,	%ax
	pushw	%ax
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	6(%bp)
	pushw	-1(%bp)
	call	_Z14PrintDiskErrorhbhPK13UINT64_STRUCTPK10ChsAddress
	addw	$10,	%sp
.L41:
	movb	-1(%bp),	%al
.L33:
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	popw	%di
	ret
	.size	_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb, .-_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb
	.type	_ZL16ReadWriteSectorsbPhhRK13UINT64_STRUCTjb, @function
_ZL16ReadWriteSectorsbPhhRK13UINT64_STRUCTjb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$16,	%sp
	movw	6(%bp),	%ax
	movw	%ax,	%ax
	cwtd
	movw	%ax,	-12(%bp)
	movw	%dx,	-10(%bp)
	pushw	14(%bp)
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	leaw	-16(%bp),	%ax
	pushw	%ax
	pushw	4(%bp)
	call	_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_ZL16ReadWriteSectorsbPhhRK13UINT64_STRUCTjb, .-_ZL16ReadWriteSectorsbPhhRK13UINT64_STRUCTjb
	.global	_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb
	.type	_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb, @function
_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$28,	%sp
	movw	6(%bp),	%ax
	movw	%ax,	%dx
	movw	$0,	%bx
	movb	$0,	%al
	movw	%dx,	%dx
	movb	%al,	%cl
	shlw	%cl,	%dx
	movw	%dx,	-18(%bp)
	movw	$0,	-20(%bp)
	movw	8(%bp),	%ax
	movw	%ax,	-24(%bp)
	movw	$0,	-22(%bp)
	movw	-20(%bp),	%ax
	orw	-24(%bp),	%ax
	movw	%ax,	-28(%bp)
	movw	-18(%bp),	%ax
	orw	-22(%bp),	%ax
	movw	%ax,	-26(%bp)
	movw	-28(%bp),	%ax
	movw	%ax,	-12(%bp)
	movw	-26(%bp),	%ax
	movw	%ax,	-10(%bp)
	pushw	16(%bp)
	pushw	14(%bp)
	pushw	12(%bp)
	pushw	10(%bp)
	leaw	-16(%bp),	%ax
	pushw	%ax
	pushw	4(%bp)
	call	_ZL16ReadWriteSectorsbR13BiosLbaPackethRK13UINT64_STRUCTjb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb, .-_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb
	.global	_Z11ReadSectorsjjhRK13UINT64_STRUCTjb
	.type	_Z11ReadSectorsjjhRK13UINT64_STRUCTjb, @function
_Z11ReadSectorsjjhRK13UINT64_STRUCTjb:
	pushw	%bp
	movw	%sp,	%bp
	pushw	14(%bp)
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	6(%bp)
	pushw	4(%bp)
	movb	$0,	%al
	pushw	%ax
	call	_Z16ReadWriteSectorsbjjhRK13UINT64_STRUCTjb
	addw	$14,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z11ReadSectorsjjhRK13UINT64_STRUCTjb, .-_Z11ReadSectorsjjhRK13UINT64_STRUCTjb
	.global	_Z11ReadSectorsPhhRK13UINT64_STRUCTjb
	.type	_Z11ReadSectorsPhhRK13UINT64_STRUCTjb, @function
_Z11ReadSectorsPhhRK13UINT64_STRUCTjb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$4,	%sp
#APP
;# 354 "BootDiskIo.cpp" 1
	
        mov %cs, %ax

;# 0 "" 2
#NO_APP
	movw	%ax,	-2(%bp)
	movw	4(%bp),	%dx
	movb	BootStarted,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L49
	movw	-2(%bp),	%ax
	jmp	.L50
.L49:
	movw	$24576,	%ax
.L50:
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	6(%bp)
	pushw	%dx
	pushw	%ax
	call	_Z11ReadSectorsjjhRK13UINT64_STRUCTjb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	-3(%bp)
	movb	BootStarted,	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L51
	movw	10(%bp),	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	movw	%ax,	%dx
	movw	4(%bp),	%ax
	pushw	%dx
	pushw	4(%bp)
	pushw	%ax
	movw	$24576,	%ax
	pushw	%ax
	call	_Z10CopyMemoryjjPvj
	addw	$8,	%sp
.L51:
	movb	-3(%bp),	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z11ReadSectorsPhhRK13UINT64_STRUCTjb, .-_Z11ReadSectorsPhhRK13UINT64_STRUCTjb
	.global	_Z12WriteSectorsPhhRK13UINT64_STRUCTjb
	.type	_Z12WriteSectorsPhhRK13UINT64_STRUCTjb, @function
_Z12WriteSectorsPhhRK13UINT64_STRUCTjb:
	pushw	%bp
	movw	%sp,	%bp
	pushw	12(%bp)
	pushw	10(%bp)
	pushw	8(%bp)
	pushw	6(%bp)
	pushw	4(%bp)
	movb	$1,	%al
	pushw	%ax
	call	_ZL16ReadWriteSectorsbPhhRK13UINT64_STRUCTjb
	addw	$12,	%sp
	movb	%al,	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z12WriteSectorsPhhRK13UINT64_STRUCTjb, .-_Z12WriteSectorsPhhRK13UINT64_STRUCTjb
	.section	.rodata
.LC7:
	.string	"Drive "
.LC8:
	.string	" not found: "
.LC9:
	.string	""
	.text
	.global	_Z16GetDriveGeometryhR13DriveGeometryb
	.type	_Z16GetDriveGeometryhR13DriveGeometryb, @function
_Z16GetDriveGeometryhR13DriveGeometryb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$12,	%sp
#APP
;# 413 "BootDiskIo.cpp" 1
	
		push %es
		mov 4(%bp), %dl
		mov $0x08, %ah
		int	$0x13

		mov	%ah, %dh
		mov %ch, %dl
		mov %cl, %ah
		mov %dh, %al
		pop %es

;# 0 "" 2
#NO_APP
	movb	%dh,	-1(%bp)
	movb	%dl,	-2(%bp)
	movb	%ah,	-3(%bp)
	movb	%al,	-4(%bp)
	cmpb	$1,	-1(%bp)
	cmpb	$0,	-1(%bp)
	jne	.L56
	movb	-2(%bp),	%al
	xorb	%ah,	%ah
	movw	%ax,	%dx
	movb	-3(%bp),	%al
	xorb	%ah,	%ah
	movw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	movw	%ax,	%ax
	andw	$768,	%ax
	orw	%dx,	%ax
	incw	%ax
	movw	6(%bp),	%bx
	movw	%ax,	(%bx)
	movb	-4(%bp),	%al
	incb	%al
	movw	6(%bp),	%bx
	movb	%al,	2(%bx)
	movb	-3(%bp),	%al
	andb	$63,	%al
	movw	6(%bp),	%bx
	movb	%al,	3(%bx)
	jmp	.L57
.L56:
	movb	8(%bp),	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L57
	movw	$.LC7,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	movb	4(%bp),	%al
	xorb	$-128,	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	%ax
	movw	%ax,	-8(%bp)
	movw	$0,	-6(%bp)
	pushw	-6(%bp)
	pushw	-8(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	movw	$.LC8,	%ax
	pushw	%ax
	call	_Z5PrintPKc
	addw	$2,	%sp
	movw	$.LC9,	%ax
	pushw	%ax
	call	_Z16PrintErrorNoEndlPKc
	addw	$2,	%sp
	movb	-1(%bp),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	%ax
	movw	%ax,	-12(%bp)
	movw	$0,	-10(%bp)
	pushw	-10(%bp)
	pushw	-12(%bp)
	call	_Z5Printm
	addw	$4,	%sp
	call	_Z9PrintEndlv
.L57:
	movb	-1(%bp),	%al
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z16GetDriveGeometryhR13DriveGeometryb, .-_Z16GetDriveGeometryhR13DriveGeometryb
	.global	_Z8ChsToLbaRK13DriveGeometryRK10ChsAddressR13UINT64_STRUCT
	.type	_Z8ChsToLbaRK13DriveGeometryRK10ChsAddressR13UINT64_STRUCT, @function
_Z8ChsToLbaRK13DriveGeometryRK10ChsAddressR13UINT64_STRUCT:
	pushw	%bp
	movw	%sp,	%bp
	subw	$32,	%sp
	movw	8(%bp),	%bx
	movw	$0,	4(%bx)
	movw	$0,	6(%bx)
	movw	6(%bp),	%bx
	movw	(%bx),	%ax
	movw	%ax,	-4(%bp)
	movw	$0,	-2(%bp)
	movw	4(%bp),	%bx
	movb	2(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-8(%bp)
	movw	$0,	-6(%bp)
	movw	-2(%bp),	%ax
	mulw	-8(%bp)
	movw	%ax,	%cx
	movw	-6(%bp),	%ax
	mulw	-4(%bp)
	addw	%ax,	%cx
	movw	-4(%bp),	%bx
	movw	%bx,	%ax
	mulw	-8(%bp)
	movw	%ax,	-4(%bp)
	movw	%dx,	-2(%bp)
	movw	%cx,	%ax
	addw	-2(%bp),	%ax
	movw	%ax,	-2(%bp)
	movw	6(%bp),	%bx
	movb	2(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-20(%bp)
	movw	$0,	-18(%bp)
	movw	-4(%bp),	%ax
	addw	-20(%bp),	%ax
	movw	%ax,	-12(%bp)
	movw	-2(%bp),	%ax
	adcw	-18(%bp),	%ax
	movw	%ax,	-10(%bp)
	movw	4(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-16(%bp)
	movw	$0,	-14(%bp)
	movw	-10(%bp),	%ax
	mulw	-16(%bp)
	movw	%ax,	%cx
	movw	-14(%bp),	%ax
	mulw	-12(%bp)
	addw	%ax,	%cx
	movw	-12(%bp),	%bx
	movw	%bx,	%ax
	mulw	-16(%bp)
	movw	%ax,	-4(%bp)
	movw	%dx,	-2(%bp)
	movw	%cx,	%ax
	addw	-2(%bp),	%ax
	movw	%ax,	-2(%bp)
	movw	6(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-24(%bp)
	movw	$0,	-22(%bp)
	movw	-4(%bp),	%ax
	addw	-24(%bp),	%ax
	movw	%ax,	-28(%bp)
	movw	-2(%bp),	%ax
	adcw	-22(%bp),	%ax
	movw	%ax,	-26(%bp)
	movw	-28(%bp),	%ax
	addw	$-1,	%ax
	movw	%ax,	-32(%bp)
	movw	-26(%bp),	%ax
	adcw	$-1,	%ax
	movw	%ax,	-30(%bp)
	movw	8(%bp),	%bx
	movw	-32(%bp),	%ax
	movw	%ax,	(%bx)
	movw	-30(%bp),	%ax
	movw	%ax,	2(%bx)
	nop
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z8ChsToLbaRK13DriveGeometryRK10ChsAddressR13UINT64_STRUCT, .-_Z8ChsToLbaRK13DriveGeometryRK10ChsAddressR13UINT64_STRUCT
	.global	_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress
	.type	_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress, @function
_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress:
	pushw	%si
	pushw	%di
	pushw	%bp
	movw	%sp,	%bp
	subw	$20,	%sp
	movw	10(%bp),	%bx
	movw	(%bx),	%si
	movw	2(%bx),	%di
	movw	8(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-8(%bp)
	movw	$0,	-6(%bp)
	pushw	-6(%bp)
	pushw	-8(%bp)
	pushw	%di
	pushw	%si
	call	__umodsi3
	addw	$8,	%sp
	movw	%ax,	%bx
	movw	%dx,	%si
	movw	%bx,	%ax
	movw	%si,	%dx
	movb	%al,	%al
	incb	%al
	movw	12(%bp),	%bx
	movb	%al,	3(%bx)
	movw	10(%bp),	%bx
	movw	(%bx),	%si
	movw	2(%bx),	%di
	movw	8(%bp),	%bx
	movb	3(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-12(%bp)
	movw	$0,	-10(%bp)
	pushw	-10(%bp)
	pushw	-12(%bp)
	pushw	%di
	pushw	%si
	call	__udivsi3
	addw	$8,	%sp
	movw	%ax,	%bx
	movw	%dx,	%si
	movw	%bx,	-4(%bp)
	movw	%si,	-2(%bp)
	movw	8(%bp),	%bx
	movb	2(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-16(%bp)
	movw	$0,	-14(%bp)
	movw	-4(%bp),	%ax
	movw	-2(%bp),	%dx
	pushw	-14(%bp)
	pushw	-16(%bp)
	pushw	%dx
	pushw	%ax
	call	__umodsi3
	addw	$8,	%sp
	movw	%ax,	%bx
	movw	%dx,	%si
	movw	%bx,	%ax
	movw	%si,	%dx
	movb	%al,	%al
	movw	12(%bp),	%bx
	movb	%al,	2(%bx)
	movw	8(%bp),	%bx
	movb	2(%bx),	%al
	movb	%al,	%al
	xorb	%ah,	%ah
	movw	%ax,	-20(%bp)
	movw	$0,	-18(%bp)
	pushw	-18(%bp)
	pushw	-20(%bp)
	pushw	-2(%bp)
	pushw	-4(%bp)
	call	__udivsi3
	addw	$8,	%sp
	movw	%ax,	%bx
	movw	%dx,	%si
	movw	%bx,	%ax
	movw	%si,	%dx
	movw	%ax,	%ax
	movw	12(%bp),	%bx
	movw	%ax,	(%bx)
	nop
	movw	%bp,	%sp
	popw	%bp
	popw	%di
	popw	%si
	ret
	.size	_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress, .-_Z8LbaToChsRK13DriveGeometryRK13UINT64_STRUCTR10ChsAddress
	.global	_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition
	.type	_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition, @function
_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition:
	pushw	%bp
	movw	%sp,	%bp
	subw	$12,	%sp
	movw	4(%bp),	%bx
	movb	(%bx),	%al
	cmpb	$-128,	%al
	lahf
	shlb	%ah
	cwtd
	movb	%dl,	%al
	movb	$0,	%ah
	subb	%al,	%ah
	movb	%ah,	%al
	movw	6(%bp),	%bx
	movb	%al,	2(%bx)
	movw	6(%bp),	%bx
	movw	$0,	8(%bx)
	movw	$0,	10(%bx)
	movw	4(%bp),	%bx
	movw	8(%bx),	%ax
	movw	%ax,	-4(%bp)
	movw	10(%bx),	%ax
	movw	%ax,	-2(%bp)
	movw	4(%bp),	%bx
	movw	12(%bx),	%ax
	movw	14(%bx),	%dx
	movw	-4(%bp),	%cx
	addw	%ax,	%cx
	movw	%cx,	-8(%bp)
	movw	%dx,	%ax
	adcw	-2(%bp),	%ax
	movw	%ax,	-6(%bp)
	movw	-8(%bp),	%ax
	addw	$-1,	%ax
	movw	%ax,	-12(%bp)
	movw	-6(%bp),	%ax
	adcw	$-1,	%ax
	movw	%ax,	-10(%bp)
	movw	6(%bp),	%bx
	movw	-12(%bp),	%ax
	movw	%ax,	4(%bx)
	movw	-10(%bp),	%ax
	movw	%ax,	6(%bx)
	movw	6(%bp),	%bx
	movw	$0,	18(%bx)
	movw	$0,	20(%bx)
	movw	4(%bp),	%bx
	movw	12(%bx),	%ax
	movw	14(%bx),	%dx
	movw	6(%bp),	%bx
	movw	%ax,	14(%bx)
	movw	%dx,	16(%bx)
	movw	6(%bp),	%bx
	movw	$0,	26(%bx)
	movw	$0,	28(%bx)
	movw	4(%bp),	%bx
	movw	8(%bx),	%ax
	movw	10(%bx),	%dx
	movw	6(%bp),	%bx
	movw	%ax,	22(%bx)
	movw	%dx,	24(%bx)
	movw	4(%bp),	%bx
	movb	4(%bx),	%al
	movw	6(%bp),	%bx
	movb	%al,	30(%bx)
	nop
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition, .-_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition
	.global	_Z12ReadWriteMBRbhb
	.type	_Z12ReadWriteMBRbhb, @function
_Z12ReadWriteMBRbhb:
	pushw	%bp
	movw	%sp,	%bp
	subw	$8,	%sp
	movw	$0,	-4(%bp)
	movw	$0,	-2(%bp)
	movw	$0,	-8(%bp)
	movw	$0,	-6(%bp)
	cmpb	$1,	4(%bp)
	cmpb	$0,	4(%bp)
	je	.L63
	pushw	8(%bp)
	movw	$1,	%ax
	pushw	%ax
	leaw	-8(%bp),	%ax
	pushw	%ax
	pushw	6(%bp)
	movw	$SectorBuffer,	%ax
	pushw	%ax
	call	_Z12WriteSectorsPhhRK13UINT64_STRUCTjb
	addw	$10,	%sp
	movb	%al,	%al
	jmp	.L65
.L63:
	pushw	8(%bp)
	movw	$1,	%ax
	pushw	%ax
	leaw	-8(%bp),	%ax
	pushw	%ax
	pushw	6(%bp)
	movw	$SectorBuffer,	%ax
	pushw	%ax
	call	_Z11ReadSectorsPhhRK13UINT64_STRUCTjb
	addw	$10,	%sp
	movb	%al,	%al
.L65:
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z12ReadWriteMBRbhb, .-_Z12ReadWriteMBRbhb
	.global	_Z18GetDrivePartitionshP9PartitionjRjbS0_b
	.type	_Z18GetDrivePartitionshP9PartitionjRjbS0_b, @function
_Z18GetDrivePartitionshP9PartitionjRjbS0_b:
	pushw	%si
	pushw	%di
	pushw	%bp
	movw	%sp,	%bp
	subw	$146,	%sp
	cmpw	$1,	18(%bp)
	cmpw	$0,	18(%bp)
	je	.L67
	movw	$255,	12(%bp)
	movw	10(%bp),	%ax
	movw	%ax,	-2(%bp)
	leaw	-50(%bp),	%ax
	movw	%ax,	10(%bp)
	movw	-2(%bp),	%bx
	movb	$127,	1(%bx)
	movw	-2(%bp),	%bx
	movw	$-1,	22(%bx)
	movw	$-1,	24(%bx)
.L67:
	pushw	20(%bp)
	pushw	8(%bp)
	movb	$0,	%al
	pushw	%ax
	call	_Z12ReadWriteMBRbhb
	addw	$6,	%sp
	movb	%al,	%al
	movb	%al,	-3(%bp)
	movw	14(%bp),	%bx
	movw	$0,	(%bx)
	movw	$SectorBuffer,	-10(%bp)
	cmpb	$1,	-3(%bp)
	cmpb	$0,	-3(%bp)
	jne	.L68
	movw	-10(%bp),	%bx
	movw	510(%bx),	%ax
	cmpw	$-21931,	%ax
	cmpw	$-21931,	%ax
	je	.L69
.L68:
	movb	-3(%bp),	%al
	jmp	.L95
.L69:
	movw	-10(%bp),	%ax
	addw	$446,	%ax
	leaw	-114(%bp),	%cx
	movw	%ax,	%dx
	movw	$64,	%ax
	pushw	%ax
	pushw	%dx
	pushw	%cx
	call	memcpy
	addw	$6,	%sp
	movw	$0,	-6(%bp)
	movw	$0,	-8(%bp)
.L94:
	cmpw	$3+1,	-8(%bp)
	cmpw	$3,	-8(%bp)
	ja	.L71
	movw	-6(%bp),	%ax
	cmpw	12(%bp),	%ax
	cmpw	12(%bp),	%ax
	jnc	.L71
	leaw	-114(%bp),	%dx
	movw	-8(%bp),	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%dx,	%ax
	movw	%ax,	-12(%bp)
	movw	-12(%bp),	%bx
	movw	12(%bx),	%ax
	movw	14(%bx),	%dx
	movw	%ax,	%cx
	orw	%dx,	%cx
	cmpw	$1,	%cx
	andw	%cx,	%cx
	je	.L72
	movw	-6(%bp),	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	movw	%ax,	%dx
	movw	10(%bp),	%ax
	addw	%dx,	%ax
	movw	%ax,	-14(%bp)
	pushw	-14(%bp)
	pushw	-12(%bp)
	call	_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition
	addw	$4,	%sp
	cmpb	$1,	16(%bp)
	cmpb	$0,	16(%bp)
	je	.L74
	movw	-14(%bp),	%bx
	movb	2(%bx),	%al
	xorb	$1,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	jne	.L100
.L74:
	movw	-14(%bp),	%bx
	movb	8(%bp),	%al
	movb	%al,	1(%bx)
	movw	-6(%bp),	%ax
	movb	%al,	%al
	movw	-14(%bp),	%bx
	movb	%al,	(%bx)
	movw	-12(%bp),	%bx
	movb	4(%bx),	%al
	cmpb	$5,	%al
	cmpb	$5,	%al
	je	.L75
	movw	-12(%bp),	%bx
	movb	4(%bx),	%al
	cmpb	$15,	%al
	cmpb	$15,	%al
	jne	.L76
.L75:
	pushw	8(%bp)
	call	_Z14IsLbaSupportedh
	addw	$2,	%sp
	movb	%al,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L101
	movw	-14(%bp),	%bx
	movw	22(%bx),	%ax
	movw	%ax,	-122(%bp)
	movw	24(%bx),	%ax
	movw	%ax,	-120(%bp)
	movw	26(%bx),	%ax
	movw	%ax,	-118(%bp)
	movw	28(%bx),	%ax
	movw	%ax,	-116(%bp)
	movw	-14(%bp),	%bx
	movw	22(%bx),	%ax
	movw	%ax,	-130(%bp)
	movw	24(%bx),	%ax
	movw	%ax,	-128(%bp)
	movw	26(%bx),	%ax
	movw	%ax,	-126(%bp)
	movw	28(%bx),	%ax
	movw	%ax,	-124(%bp)
	movw	$SectorBuffer,	-16(%bp)
.L89:
	movw	-6(%bp),	%ax
	cmpw	12(%bp),	%ax
	cmpw	12(%bp),	%ax
	jnc	.L78
	pushw	20(%bp)
	movw	$1,	%ax
	pushw	%ax
	leaw	-130(%bp),	%ax
	pushw	%ax
	pushw	8(%bp)
	pushw	-16(%bp)
	call	_Z11ReadSectorsPhhRK13UINT64_STRUCTjb
	addw	$10,	%sp
	movb	%al,	%al
	movb	%al,	-3(%bp)
	cmpb	$1,	-3(%bp)
	cmpb	$0,	-3(%bp)
	jne	.L78
	movw	-16(%bp),	%bx
	movw	510(%bx),	%ax
	cmpw	$-21931,	%ax
	cmpw	$-21931,	%ax
	jne	.L78
	movb	$1,	%al
	jmp	.L79
.L78:
	movb	$0,	%al
.L79:
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L101
	movw	-16(%bp),	%bx
	movw	458(%bx),	%ax
	movw	460(%bx),	%dx
	movw	%ax,	%cx
	orw	%dx,	%cx
	cmpw	$1,	%cx
	andw	%cx,	%cx
	je	.L81
	movw	-6(%bp),	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	addw	%ax,	%ax
	movw	%ax,	%dx
	movw	10(%bp),	%ax
	addw	%dx,	%ax
	movw	%ax,	-18(%bp)
	movw	-16(%bp),	%ax
	addw	$446,	%ax
	pushw	-18(%bp)
	pushw	%ax
	call	_Z28PartitionEntryMBRToPartitionRK17PartitionEntryMBRR9Partition
	addw	$4,	%sp
	movw	-18(%bp),	%bx
	movb	8(%bp),	%al
	movb	%al,	1(%bx)
	movw	-6(%bp),	%ax
	movb	%al,	%al
	movw	-18(%bp),	%bx
	movb	%al,	(%bx)
	movw	-18(%bp),	%bx
	movb	$0,	12(%bx)
	movw	-18(%bp),	%di
	movw	22(%di),	%bx
	movw	24(%di),	%si
	movw	-130(%bp),	%ax
	movw	-128(%bp),	%dx
	movw	%bx,	%cx
	addw	%ax,	%cx
	movw	%cx,	-138(%bp)
	movw	%si,	%cx
	adcw	%dx,	%cx
	movw	%cx,	-136(%bp)
	movw	-18(%bp),	%bx
	movw	-138(%bp),	%ax
	movw	%ax,	22(%bx)
	movw	-136(%bp),	%ax
	movw	%ax,	24(%bx)
	movw	-18(%bp),	%di
	movw	4(%di),	%bx
	movw	6(%di),	%si
	movw	-130(%bp),	%ax
	movw	-128(%bp),	%dx
	movw	%bx,	%cx
	addw	%ax,	%cx
	movw	%cx,	-142(%bp)
	movw	%si,	%cx
	adcw	%dx,	%cx
	movw	%cx,	-140(%bp)
	movw	-18(%bp),	%bx
	movw	-142(%bp),	%ax
	movw	%ax,	4(%bx)
	movw	-140(%bp),	%ax
	movw	%ax,	6(%bx)
	cmpw	$1,	18(%bp)
	cmpw	$0,	18(%bp)
	je	.L83
	movw	-18(%bp),	%bx
	movw	22(%bx),	%ax
	movw	%ax,	-134(%bp)
	movw	24(%bx),	%ax
	movw	%ax,	-132(%bp)
	movw	18(%bp),	%bx
	movw	4(%bx),	%ax
	movw	6(%bx),	%dx
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	ja	.L96
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	jne	.L81
	cmpw	%ax,	-134(%bp)
	cmpw	%ax,	-134(%bp)
	jbe	.L81
.L96:
	movw	-18(%bp),	%bx
	movw	22(%bx),	%ax
	movw	24(%bx),	%dx
	movw	-2(%bp),	%bx
	movw	22(%bx),	%cx
	movw	%cx,	-134(%bp)
	movw	24(%bx),	%cx
	movw	%cx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	ja	.L97
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	jne	.L81
	cmpw	%ax,	-134(%bp)
	cmpw	%ax,	-134(%bp)
	jbe	.L81
.L97:
	movw	-2(%bp),	%dx
	movw	-18(%bp),	%ax
	movw	%dx,	%cx
	movw	%ax,	%dx
	movw	$32,	%ax
	pushw	%ax
	pushw	%dx
	pushw	%cx
	call	memcpy
	addw	$6,	%sp
	jmp	.L81
.L83:
	movw	-6(%bp),	%ax
	incw	%ax
	movw	%ax,	-6(%bp)
.L81:
	movw	-16(%bp),	%bx
	movb	466(%bx),	%al
	cmpb	$5,	%al
	cmpb	$5,	%al
	je	.L87
	movw	-16(%bp),	%bx
	movb	466(%bx),	%al
	cmpb	$15,	%al
	cmpb	$15,	%al
	je	.L101
.L87:
	movw	-16(%bp),	%bx
	movw	474(%bx),	%ax
	movw	476(%bx),	%dx
	movw	%ax,	%cx
	orw	%dx,	%cx
	cmpw	$1,	%cx
	andw	%cx,	%cx
	je	.L101
	movw	-16(%bp),	%di
	movw	470(%di),	%bx
	movw	472(%di),	%si
	movw	-122(%bp),	%ax
	movw	-120(%bp),	%dx
	movw	%bx,	%cx
	addw	%ax,	%cx
	movw	%cx,	-146(%bp)
	movw	%si,	%cx
	adcw	%dx,	%cx
	movw	%cx,	-144(%bp)
	movw	-146(%bp),	%ax
	movw	%ax,	-130(%bp)
	movw	-144(%bp),	%ax
	movw	%ax,	-128(%bp)
	jmp	.L89
.L76:
	movw	-14(%bp),	%bx
	movb	$1,	12(%bx)
	cmpw	$1,	18(%bp)
	cmpw	$0,	18(%bp)
	je	.L91
	movw	-14(%bp),	%bx
	movw	22(%bx),	%ax
	movw	%ax,	-134(%bp)
	movw	24(%bx),	%ax
	movw	%ax,	-132(%bp)
	movw	18(%bp),	%bx
	movw	4(%bx),	%ax
	movw	6(%bx),	%dx
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	ja	.L98
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	jne	.L72
	cmpw	%ax,	-134(%bp)
	cmpw	%ax,	-134(%bp)
	jbe	.L72
.L98:
	movw	-14(%bp),	%bx
	movw	22(%bx),	%ax
	movw	24(%bx),	%dx
	movw	-2(%bp),	%bx
	movw	22(%bx),	%cx
	movw	%cx,	-134(%bp)
	movw	24(%bx),	%cx
	movw	%cx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	ja	.L99
	cmpw	%dx,	-132(%bp)
	cmpw	%dx,	-132(%bp)
	jne	.L72
	cmpw	%ax,	-134(%bp)
	cmpw	%ax,	-134(%bp)
	jbe	.L72
.L99:
	movw	-2(%bp),	%dx
	movw	-14(%bp),	%ax
	movw	%dx,	%cx
	movw	%ax,	%dx
	movw	$32,	%ax
	pushw	%ax
	pushw	%dx
	pushw	%cx
	call	memcpy
	addw	$6,	%sp
	jmp	.L72
.L91:
	movw	-6(%bp),	%ax
	incw	%ax
	movw	%ax,	-6(%bp)
	jmp	.L72
.L100:
	nop
	jmp	.L72
.L101:
	nop
.L72:
	movw	-8(%bp),	%ax
	incw	%ax
	movw	%ax,	-8(%bp)
	jmp	.L94
.L71:
	movw	14(%bp),	%bx
	movw	-6(%bp),	%ax
	movw	%ax,	(%bx)
	movb	-3(%bp),	%al
.L95:
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	popw	%di
	popw	%si
	ret
	.size	_Z18GetDrivePartitionshP9PartitionjRjbS0_b, .-_Z18GetDrivePartitionshP9PartitionjRjbS0_b
	.section	.rodata
.LC10:
	.string	"No bootable partition found"
	.text
	.global	_Z18GetActivePartitionh
	.type	_Z18GetActivePartitionh, @function
_Z18GetActivePartitionh:
	pushw	%bp
	movw	%sp,	%bp
	subw	$2,	%sp
	movb	$0,	%al
	pushw	%ax
	movw	$0,	%ax
	pushw	%ax
	movb	$1,	%al
	pushw	%ax
	leaw	-2(%bp),	%ax
	pushw	%ax
	movw	$1,	%ax
	pushw	%ax
	movw	$ActivePartition,	%ax
	pushw	%ax
	pushw	4(%bp)
	call	_Z18GetDrivePartitionshP9PartitionjRjbS0_b
	addw	$14,	%sp
	movb	%al,	%al
	cmpb	$1,	%al
	andb	%al,	%al
	jne	.L103
	movw	-2(%bp),	%ax
	cmpw	$1,	%ax
	andw	%ax,	%ax
	jne	.L104
.L103:
	movb	$1,	%al
	jmp	.L105
.L104:
	movb	$0,	%al
.L105:
	cmpb	$1,	%al
	andb	%al,	%al
	je	.L106
	movb	$127,	ActivePartition+1
	movw	$.LC10,	%ax
	pushw	%ax
	call	_Z10PrintErrorPKc
	addw	$2,	%sp
	movb	$0,	%al
	jmp	.L108
.L106:
	movb	$1,	%al
.L108:
	movb	%al,	%al
	movb	%al,	%al
	movw	%bp,	%sp
	popw	%bp
	ret
	.size	_Z18GetActivePartitionh, .-_Z18GetActivePartitionh
	.ident	"GCC: (Sourcery CodeBench Lite 2016.11-64) 6.2.0"
