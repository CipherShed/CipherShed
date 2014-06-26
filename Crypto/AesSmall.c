/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2006, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software is allowed (with or without
 changes) provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 09/09/2006

 This is an AES implementation that uses only 8-bit byte operations on the
 cipher state (there are options to use 32-bit types if available).

 The combination of mix columns and byte substitution used here is based on
 that developed by Karl Malbrain. His contribution is acknowledged.
 */

/* Adapted for CipherShed:
  - Macro-generated tables were replaced with static data to enable compiling
    with MSVC++ 1.5 which runs out of resources when expanding large macros.
*/

#pragma optimize ("t", on)

/* define if you have a fast memcpy function on your system */
#if 1
#  define HAVE_MEMCPY
#  include <string.h>
#  if defined( _MSC_VER )
#    ifndef DEBUG
#      pragma intrinsic( memcpy )
#    endif
#  endif
#endif

/* define if you have fast 32-bit types on your system */
#if 1
#  define HAVE_UINT_32T
#endif

/* alternative versions (test for performance on your system) */
#if 0
#  define VERSION_1
#endif

#include "AesSmall.h"

#define WPOLY   0x011b
#define DPOLY   0x008d
#define f1(x)   (x)
#define f2(x)   ((x<<1) ^ (((x>>7) & 1) * WPOLY))
#define f4(x)   ((x<<2) ^ (((x>>6) & 1) * WPOLY) ^ (((x>>6) & 2) * WPOLY))
#define f8(x)   ((x<<3) ^ (((x>>5) & 1) * WPOLY) ^ (((x>>5) & 2) * WPOLY) \
                        ^ (((x>>5) & 4) * WPOLY))
#define d2(x)   (((x) >> 1) ^ ((x) & 1 ? DPOLY : 0))

#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

static const uint_8t s_box[256] = {
	0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,
	0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
	0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,
	0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
	0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,
	0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
	0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,
	0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
	0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,
	0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
	0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,
	0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
	0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,
	0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
	0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,
	0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
	0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,
	0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
	0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,
	0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
	0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,
	0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
	0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,
	0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
	0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,
	0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
	0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,
	0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
	0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,
	0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
	0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,
	0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static const uint_8t inv_s_box[256] = {
	0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,
	0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
	0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,
	0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
	0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,
	0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
	0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,
	0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
	0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,
	0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
	0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,
	0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
	0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,
	0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
	0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,
	0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
	0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,
	0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
	0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,
	0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
	0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,
	0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
	0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,
	0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
	0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,
	0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
	0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,
	0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
	0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,
	0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
	0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,
	0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

static const uint_8t gfm2_s_box[256] = {
	0xc6,0xf8,0xee,0xf6,0xff,0xd6,0xde,0x91,
	0x60,0x02,0xce,0x56,0xe7,0xb5,0x4d,0xec,
	0x8f,0x1f,0x89,0xfa,0xef,0xb2,0x8e,0xfb,
	0x41,0xb3,0x5f,0x45,0x23,0x53,0xe4,0x9b,
	0x75,0xe1,0x3d,0x4c,0x6c,0x7e,0xf5,0x83,
	0x68,0x51,0xd1,0xf9,0xe2,0xab,0x62,0x2a,
	0x08,0x95,0x46,0x9d,0x30,0x37,0x0a,0x2f,
	0x0e,0x24,0x1b,0xdf,0xcd,0x4e,0x7f,0xea,
	0x12,0x1d,0x58,0x34,0x36,0xdc,0xb4,0x5b,
	0xa4,0x76,0xb7,0x7d,0x52,0xdd,0x5e,0x13,
	0xa6,0xb9,0x00,0xc1,0x40,0xe3,0x79,0xb6,
	0xd4,0x8d,0x67,0x72,0x94,0x98,0xb0,0x85,
	0xbb,0xc5,0x4f,0xed,0x86,0x9a,0x66,0x11,
	0x8a,0xe9,0x04,0xfe,0xa0,0x78,0x25,0x4b,
	0xa2,0x5d,0x80,0x05,0x3f,0x21,0x70,0xf1,
	0x63,0x77,0xaf,0x42,0x20,0xe5,0xfd,0xbf,
	0x81,0x18,0x26,0xc3,0xbe,0x35,0x88,0x2e,
	0x93,0x55,0xfc,0x7a,0xc8,0xba,0x32,0xe6,
	0xc0,0x19,0x9e,0xa3,0x44,0x54,0x3b,0x0b,
	0x8c,0xc7,0x6b,0x28,0xa7,0xbc,0x16,0xad,
	0xdb,0x64,0x74,0x14,0x92,0x0c,0x48,0xb8,
	0x9f,0xbd,0x43,0xc4,0x39,0x31,0xd3,0xf2,
	0xd5,0x8b,0x6e,0xda,0x01,0xb1,0x9c,0x49,
	0xd8,0xac,0xf3,0xcf,0xca,0xf4,0x47,0x10,
	0x6f,0xf0,0x4a,0x5c,0x38,0x57,0x73,0x97,
	0xcb,0xa1,0xe8,0x3e,0x96,0x61,0x0d,0x0f,
	0xe0,0x7c,0x71,0xcc,0x90,0x06,0xf7,0x1c,
	0xc2,0x6a,0xae,0x69,0x17,0x99,0x3a,0x27,
	0xd9,0xeb,0x2b,0x22,0xd2,0xa9,0x07,0x33,
	0x2d,0x3c,0x15,0xc9,0x87,0xaa,0x50,0xa5,
	0x03,0x59,0x09,0x1a,0x65,0xd7,0x84,0xd0,
	0x82,0x29,0x5a,0x1e,0x7b,0xa8,0x6d,0x2c
};

static const uint_8t gfm3_s_box[256] = {
	0xa5,0x84,0x99,0x8d,0x0d,0xbd,0xb1,0x54,
	0x50,0x03,0xa9,0x7d,0x19,0x62,0xe6,0x9a,
	0x45,0x9d,0x40,0x87,0x15,0xeb,0xc9,0x0b,
	0xec,0x67,0xfd,0xea,0xbf,0xf7,0x96,0x5b,
	0xc2,0x1c,0xae,0x6a,0x5a,0x41,0x02,0x4f,
	0x5c,0xf4,0x34,0x08,0x93,0x73,0x53,0x3f,
	0x0c,0x52,0x65,0x5e,0x28,0xa1,0x0f,0xb5,
	0x09,0x36,0x9b,0x3d,0x26,0x69,0xcd,0x9f,
	0x1b,0x9e,0x74,0x2e,0x2d,0xb2,0xee,0xfb,
	0xf6,0x4d,0x61,0xce,0x7b,0x3e,0x71,0x97,
	0xf5,0x68,0x00,0x2c,0x60,0x1f,0xc8,0xed,
	0xbe,0x46,0xd9,0x4b,0xde,0xd4,0xe8,0x4a,
	0x6b,0x2a,0xe5,0x16,0xc5,0xd7,0x55,0x94,
	0xcf,0x10,0x06,0x81,0xf0,0x44,0xba,0xe3,
	0xf3,0xfe,0xc0,0x8a,0xad,0xbc,0x48,0x04,
	0xdf,0xc1,0x75,0x63,0x30,0x1a,0x0e,0x6d,
	0x4c,0x14,0x35,0x2f,0xe1,0xa2,0xcc,0x39,
	0x57,0xf2,0x82,0x47,0xac,0xe7,0x2b,0x95,
	0xa0,0x98,0xd1,0x7f,0x66,0x7e,0xab,0x83,
	0xca,0x29,0xd3,0x3c,0x79,0xe2,0x1d,0x76,
	0x3b,0x56,0x4e,0x1e,0xdb,0x0a,0x6c,0xe4,
	0x5d,0x6e,0xef,0xa6,0xa8,0xa4,0x37,0x8b,
	0x32,0x43,0x59,0xb7,0x8c,0x64,0xd2,0xe0,
	0xb4,0xfa,0x07,0x25,0xaf,0x8e,0xe9,0x18,
	0xd5,0x88,0x6f,0x72,0x24,0xf1,0xc7,0x51,
	0x23,0x7c,0x9c,0x21,0xdd,0xdc,0x86,0x85,
	0x90,0x42,0xc4,0xaa,0xd8,0x05,0x01,0x12,
	0xa3,0x5f,0xf9,0xd0,0x91,0x58,0x27,0xb9,
	0x38,0x13,0xb3,0x33,0xbb,0x70,0x89,0xa7,
	0xb6,0x22,0x92,0x20,0x49,0xff,0x78,0x7a,
	0x8f,0xf8,0x80,0x17,0xda,0x31,0xc6,0xb8,
	0xc3,0xb0,0x77,0x11,0xcb,0xfc,0xd6,0x3a
};

static const uint_8t gfmul_9[256] = {
	0x00,0x09,0x12,0x1b,0x24,0x2d,0x36,0x3f,
	0x48,0x41,0x5a,0x53,0x6c,0x65,0x7e,0x77,
	0x90,0x99,0x82,0x8b,0xb4,0xbd,0xa6,0xaf,
	0xd8,0xd1,0xca,0xc3,0xfc,0xf5,0xee,0xe7,
	0x3b,0x32,0x29,0x20,0x1f,0x16,0x0d,0x04,
	0x73,0x7a,0x61,0x68,0x57,0x5e,0x45,0x4c,
	0xab,0xa2,0xb9,0xb0,0x8f,0x86,0x9d,0x94,
	0xe3,0xea,0xf1,0xf8,0xc7,0xce,0xd5,0xdc,
	0x76,0x7f,0x64,0x6d,0x52,0x5b,0x40,0x49,
	0x3e,0x37,0x2c,0x25,0x1a,0x13,0x08,0x01,
	0xe6,0xef,0xf4,0xfd,0xc2,0xcb,0xd0,0xd9,
	0xae,0xa7,0xbc,0xb5,0x8a,0x83,0x98,0x91,
	0x4d,0x44,0x5f,0x56,0x69,0x60,0x7b,0x72,
	0x05,0x0c,0x17,0x1e,0x21,0x28,0x33,0x3a,
	0xdd,0xd4,0xcf,0xc6,0xf9,0xf0,0xeb,0xe2,
	0x95,0x9c,0x87,0x8e,0xb1,0xb8,0xa3,0xaa,
	0xec,0xe5,0xfe,0xf7,0xc8,0xc1,0xda,0xd3,
	0xa4,0xad,0xb6,0xbf,0x80,0x89,0x92,0x9b,
	0x7c,0x75,0x6e,0x67,0x58,0x51,0x4a,0x43,
	0x34,0x3d,0x26,0x2f,0x10,0x19,0x02,0x0b,
	0xd7,0xde,0xc5,0xcc,0xf3,0xfa,0xe1,0xe8,
	0x9f,0x96,0x8d,0x84,0xbb,0xb2,0xa9,0xa0,
	0x47,0x4e,0x55,0x5c,0x63,0x6a,0x71,0x78,
	0x0f,0x06,0x1d,0x14,0x2b,0x22,0x39,0x30,
	0x9a,0x93,0x88,0x81,0xbe,0xb7,0xac,0xa5,
	0xd2,0xdb,0xc0,0xc9,0xf6,0xff,0xe4,0xed,
	0x0a,0x03,0x18,0x11,0x2e,0x27,0x3c,0x35,
	0x42,0x4b,0x50,0x59,0x66,0x6f,0x74,0x7d,
	0xa1,0xa8,0xb3,0xba,0x85,0x8c,0x97,0x9e,
	0xe9,0xe0,0xfb,0xf2,0xcd,0xc4,0xdf,0xd6,
	0x31,0x38,0x23,0x2a,0x15,0x1c,0x07,0x0e,
	0x79,0x70,0x6b,0x62,0x5d,0x54,0x4f,0x46
};

static const uint_8t gfmul_b[256] = {
	0x00,0x0b,0x16,0x1d,0x2c,0x27,0x3a,0x31,
	0x58,0x53,0x4e,0x45,0x74,0x7f,0x62,0x69,
	0xb0,0xbb,0xa6,0xad,0x9c,0x97,0x8a,0x81,
	0xe8,0xe3,0xfe,0xf5,0xc4,0xcf,0xd2,0xd9,
	0x7b,0x70,0x6d,0x66,0x57,0x5c,0x41,0x4a,
	0x23,0x28,0x35,0x3e,0x0f,0x04,0x19,0x12,
	0xcb,0xc0,0xdd,0xd6,0xe7,0xec,0xf1,0xfa,
	0x93,0x98,0x85,0x8e,0xbf,0xb4,0xa9,0xa2,
	0xf6,0xfd,0xe0,0xeb,0xda,0xd1,0xcc,0xc7,
	0xae,0xa5,0xb8,0xb3,0x82,0x89,0x94,0x9f,
	0x46,0x4d,0x50,0x5b,0x6a,0x61,0x7c,0x77,
	0x1e,0x15,0x08,0x03,0x32,0x39,0x24,0x2f,
	0x8d,0x86,0x9b,0x90,0xa1,0xaa,0xb7,0xbc,
	0xd5,0xde,0xc3,0xc8,0xf9,0xf2,0xef,0xe4,
	0x3d,0x36,0x2b,0x20,0x11,0x1a,0x07,0x0c,
	0x65,0x6e,0x73,0x78,0x49,0x42,0x5f,0x54,
	0xf7,0xfc,0xe1,0xea,0xdb,0xd0,0xcd,0xc6,
	0xaf,0xa4,0xb9,0xb2,0x83,0x88,0x95,0x9e,
	0x47,0x4c,0x51,0x5a,0x6b,0x60,0x7d,0x76,
	0x1f,0x14,0x09,0x02,0x33,0x38,0x25,0x2e,
	0x8c,0x87,0x9a,0x91,0xa0,0xab,0xb6,0xbd,
	0xd4,0xdf,0xc2,0xc9,0xf8,0xf3,0xee,0xe5,
	0x3c,0x37,0x2a,0x21,0x10,0x1b,0x06,0x0d,
	0x64,0x6f,0x72,0x79,0x48,0x43,0x5e,0x55,
	0x01,0x0a,0x17,0x1c,0x2d,0x26,0x3b,0x30,
	0x59,0x52,0x4f,0x44,0x75,0x7e,0x63,0x68,
	0xb1,0xba,0xa7,0xac,0x9d,0x96,0x8b,0x80,
	0xe9,0xe2,0xff,0xf4,0xc5,0xce,0xd3,0xd8,
	0x7a,0x71,0x6c,0x67,0x56,0x5d,0x40,0x4b,
	0x22,0x29,0x34,0x3f,0x0e,0x05,0x18,0x13,
	0xca,0xc1,0xdc,0xd7,0xe6,0xed,0xf0,0xfb,
	0x92,0x99,0x84,0x8f,0xbe,0xb5,0xa8,0xa3
};

static const uint_8t gfmul_d[256] = {
	0x00,0x0d,0x1a,0x17,0x34,0x39,0x2e,0x23,
	0x68,0x65,0x72,0x7f,0x5c,0x51,0x46,0x4b,
	0xd0,0xdd,0xca,0xc7,0xe4,0xe9,0xfe,0xf3,
	0xb8,0xb5,0xa2,0xaf,0x8c,0x81,0x96,0x9b,
	0xbb,0xb6,0xa1,0xac,0x8f,0x82,0x95,0x98,
	0xd3,0xde,0xc9,0xc4,0xe7,0xea,0xfd,0xf0,
	0x6b,0x66,0x71,0x7c,0x5f,0x52,0x45,0x48,
	0x03,0x0e,0x19,0x14,0x37,0x3a,0x2d,0x20,
	0x6d,0x60,0x77,0x7a,0x59,0x54,0x43,0x4e,
	0x05,0x08,0x1f,0x12,0x31,0x3c,0x2b,0x26,
	0xbd,0xb0,0xa7,0xaa,0x89,0x84,0x93,0x9e,
	0xd5,0xd8,0xcf,0xc2,0xe1,0xec,0xfb,0xf6,
	0xd6,0xdb,0xcc,0xc1,0xe2,0xef,0xf8,0xf5,
	0xbe,0xb3,0xa4,0xa9,0x8a,0x87,0x90,0x9d,
	0x06,0x0b,0x1c,0x11,0x32,0x3f,0x28,0x25,
	0x6e,0x63,0x74,0x79,0x5a,0x57,0x40,0x4d,
	0xda,0xd7,0xc0,0xcd,0xee,0xe3,0xf4,0xf9,
	0xb2,0xbf,0xa8,0xa5,0x86,0x8b,0x9c,0x91,
	0x0a,0x07,0x10,0x1d,0x3e,0x33,0x24,0x29,
	0x62,0x6f,0x78,0x75,0x56,0x5b,0x4c,0x41,
	0x61,0x6c,0x7b,0x76,0x55,0x58,0x4f,0x42,
	0x09,0x04,0x13,0x1e,0x3d,0x30,0x27,0x2a,
	0xb1,0xbc,0xab,0xa6,0x85,0x88,0x9f,0x92,
	0xd9,0xd4,0xc3,0xce,0xed,0xe0,0xf7,0xfa,
	0xb7,0xba,0xad,0xa0,0x83,0x8e,0x99,0x94,
	0xdf,0xd2,0xc5,0xc8,0xeb,0xe6,0xf1,0xfc,
	0x67,0x6a,0x7d,0x70,0x53,0x5e,0x49,0x44,
	0x0f,0x02,0x15,0x18,0x3b,0x36,0x21,0x2c,
	0x0c,0x01,0x16,0x1b,0x38,0x35,0x22,0x2f,
	0x64,0x69,0x7e,0x73,0x50,0x5d,0x4a,0x47,
	0xdc,0xd1,0xc6,0xcb,0xe8,0xe5,0xf2,0xff,
	0xb4,0xb9,0xae,0xa3,0x80,0x8d,0x9a,0x97
};

static const uint_8t gfmul_e[256] = {
	0x00,0x0e,0x1c,0x12,0x38,0x36,0x24,0x2a,
	0x70,0x7e,0x6c,0x62,0x48,0x46,0x54,0x5a,
	0xe0,0xee,0xfc,0xf2,0xd8,0xd6,0xc4,0xca,
	0x90,0x9e,0x8c,0x82,0xa8,0xa6,0xb4,0xba,
	0xdb,0xd5,0xc7,0xc9,0xe3,0xed,0xff,0xf1,
	0xab,0xa5,0xb7,0xb9,0x93,0x9d,0x8f,0x81,
	0x3b,0x35,0x27,0x29,0x03,0x0d,0x1f,0x11,
	0x4b,0x45,0x57,0x59,0x73,0x7d,0x6f,0x61,
	0xad,0xa3,0xb1,0xbf,0x95,0x9b,0x89,0x87,
	0xdd,0xd3,0xc1,0xcf,0xe5,0xeb,0xf9,0xf7,
	0x4d,0x43,0x51,0x5f,0x75,0x7b,0x69,0x67,
	0x3d,0x33,0x21,0x2f,0x05,0x0b,0x19,0x17,
	0x76,0x78,0x6a,0x64,0x4e,0x40,0x52,0x5c,
	0x06,0x08,0x1a,0x14,0x3e,0x30,0x22,0x2c,
	0x96,0x98,0x8a,0x84,0xae,0xa0,0xb2,0xbc,
	0xe6,0xe8,0xfa,0xf4,0xde,0xd0,0xc2,0xcc,
	0x41,0x4f,0x5d,0x53,0x79,0x77,0x65,0x6b,
	0x31,0x3f,0x2d,0x23,0x09,0x07,0x15,0x1b,
	0xa1,0xaf,0xbd,0xb3,0x99,0x97,0x85,0x8b,
	0xd1,0xdf,0xcd,0xc3,0xe9,0xe7,0xf5,0xfb,
	0x9a,0x94,0x86,0x88,0xa2,0xac,0xbe,0xb0,
	0xea,0xe4,0xf6,0xf8,0xd2,0xdc,0xce,0xc0,
	0x7a,0x74,0x66,0x68,0x42,0x4c,0x5e,0x50,
	0x0a,0x04,0x16,0x18,0x32,0x3c,0x2e,0x20,
	0xec,0xe2,0xf0,0xfe,0xd4,0xda,0xc8,0xc6,
	0x9c,0x92,0x80,0x8e,0xa4,0xaa,0xb8,0xb6,
	0x0c,0x02,0x10,0x1e,0x34,0x3a,0x28,0x26,
	0x7c,0x72,0x60,0x6e,0x44,0x4a,0x58,0x56,
	0x37,0x39,0x2b,0x25,0x0f,0x01,0x13,0x1d,
	0x47,0x49,0x5b,0x55,0x7f,0x71,0x63,0x6d,
	0xd7,0xd9,0xcb,0xc5,0xef,0xe1,0xf3,0xfd,
	0xa7,0xa9,0xbb,0xb5,0x9f,0x91,0x83,0x8d
};

#if defined( HAVE_UINT_32T )
  typedef unsigned long uint_32t;
#endif

#if defined( HAVE_MEMCPY )
#  define block_copy(d, s, l) memcpy(d, s, l)
#  define block16_copy(d, s)  memcpy(d, s, N_BLOCK)
#else
#  define block_copy(d, s, l) copy_block(d, s, l)
#  define block16_copy(d, s)  copy_block16(d, s)
#endif

/* block size 'nn' must be a multiple of four */

static void copy_block16( void *d, const void *s )
{
#if defined( HAVE_UINT_32T )
    ((uint_32t*)d)[ 0] = ((uint_32t*)s)[ 0];
    ((uint_32t*)d)[ 1] = ((uint_32t*)s)[ 1];
    ((uint_32t*)d)[ 2] = ((uint_32t*)s)[ 2];
    ((uint_32t*)d)[ 3] = ((uint_32t*)s)[ 3];
#else
    ((uint_8t*)d)[ 0] = ((uint_8t*)s)[ 0];
    ((uint_8t*)d)[ 1] = ((uint_8t*)s)[ 1];
    ((uint_8t*)d)[ 2] = ((uint_8t*)s)[ 2];
    ((uint_8t*)d)[ 3] = ((uint_8t*)s)[ 3];
    ((uint_8t*)d)[ 4] = ((uint_8t*)s)[ 4];
    ((uint_8t*)d)[ 5] = ((uint_8t*)s)[ 5];
    ((uint_8t*)d)[ 6] = ((uint_8t*)s)[ 6];
    ((uint_8t*)d)[ 7] = ((uint_8t*)s)[ 7];
    ((uint_8t*)d)[ 8] = ((uint_8t*)s)[ 8];
    ((uint_8t*)d)[ 9] = ((uint_8t*)s)[ 9];
    ((uint_8t*)d)[10] = ((uint_8t*)s)[10];
    ((uint_8t*)d)[11] = ((uint_8t*)s)[11];
    ((uint_8t*)d)[12] = ((uint_8t*)s)[12];
    ((uint_8t*)d)[13] = ((uint_8t*)s)[13];
    ((uint_8t*)d)[14] = ((uint_8t*)s)[14];
    ((uint_8t*)d)[15] = ((uint_8t*)s)[15];
#endif
}

static void copy_block( void * d, void *s, uint_8t nn )
{
    while( nn-- )
        *((uint_8t*)d)++ = *((uint_8t*)s)++;
}

static void xor_block( void *d, const void *s )
{
#if defined( HAVE_UINT_32T )
    ((uint_32t*)d)[ 0] ^= ((uint_32t*)s)[ 0];
    ((uint_32t*)d)[ 1] ^= ((uint_32t*)s)[ 1];
    ((uint_32t*)d)[ 2] ^= ((uint_32t*)s)[ 2];
    ((uint_32t*)d)[ 3] ^= ((uint_32t*)s)[ 3];
#else
    ((uint_8t*)d)[ 0] ^= ((uint_8t*)s)[ 0];
    ((uint_8t*)d)[ 1] ^= ((uint_8t*)s)[ 1];
    ((uint_8t*)d)[ 2] ^= ((uint_8t*)s)[ 2];
    ((uint_8t*)d)[ 3] ^= ((uint_8t*)s)[ 3];
    ((uint_8t*)d)[ 4] ^= ((uint_8t*)s)[ 4];
    ((uint_8t*)d)[ 5] ^= ((uint_8t*)s)[ 5];
    ((uint_8t*)d)[ 6] ^= ((uint_8t*)s)[ 6];
    ((uint_8t*)d)[ 7] ^= ((uint_8t*)s)[ 7];
    ((uint_8t*)d)[ 8] ^= ((uint_8t*)s)[ 8];
    ((uint_8t*)d)[ 9] ^= ((uint_8t*)s)[ 9];
    ((uint_8t*)d)[10] ^= ((uint_8t*)s)[10];
    ((uint_8t*)d)[11] ^= ((uint_8t*)s)[11];
    ((uint_8t*)d)[12] ^= ((uint_8t*)s)[12];
    ((uint_8t*)d)[13] ^= ((uint_8t*)s)[13];
    ((uint_8t*)d)[14] ^= ((uint_8t*)s)[14];
    ((uint_8t*)d)[15] ^= ((uint_8t*)s)[15];
#endif
}

static void copy_and_key( void *d, const void *s, const void *k )
{
#if defined( HAVE_UINT_32T )
    ((uint_32t*)d)[ 0] = ((uint_32t*)s)[ 0] ^ ((uint_32t*)k)[ 0];
    ((uint_32t*)d)[ 1] = ((uint_32t*)s)[ 1] ^ ((uint_32t*)k)[ 1];
    ((uint_32t*)d)[ 2] = ((uint_32t*)s)[ 2] ^ ((uint_32t*)k)[ 2];
    ((uint_32t*)d)[ 3] = ((uint_32t*)s)[ 3] ^ ((uint_32t*)k)[ 3];
#elif 1
    ((uint_8t*)d)[ 0] = ((uint_8t*)s)[ 0] ^ ((uint_8t*)k)[ 0];
    ((uint_8t*)d)[ 1] = ((uint_8t*)s)[ 1] ^ ((uint_8t*)k)[ 1];
    ((uint_8t*)d)[ 2] = ((uint_8t*)s)[ 2] ^ ((uint_8t*)k)[ 2];
    ((uint_8t*)d)[ 3] = ((uint_8t*)s)[ 3] ^ ((uint_8t*)k)[ 3];
    ((uint_8t*)d)[ 4] = ((uint_8t*)s)[ 4] ^ ((uint_8t*)k)[ 4];
    ((uint_8t*)d)[ 5] = ((uint_8t*)s)[ 5] ^ ((uint_8t*)k)[ 5];
    ((uint_8t*)d)[ 6] = ((uint_8t*)s)[ 6] ^ ((uint_8t*)k)[ 6];
    ((uint_8t*)d)[ 7] = ((uint_8t*)s)[ 7] ^ ((uint_8t*)k)[ 7];
    ((uint_8t*)d)[ 8] = ((uint_8t*)s)[ 8] ^ ((uint_8t*)k)[ 8];
    ((uint_8t*)d)[ 9] = ((uint_8t*)s)[ 9] ^ ((uint_8t*)k)[ 9];
    ((uint_8t*)d)[10] = ((uint_8t*)s)[10] ^ ((uint_8t*)k)[10];
    ((uint_8t*)d)[11] = ((uint_8t*)s)[11] ^ ((uint_8t*)k)[11];
    ((uint_8t*)d)[12] = ((uint_8t*)s)[12] ^ ((uint_8t*)k)[12];
    ((uint_8t*)d)[13] = ((uint_8t*)s)[13] ^ ((uint_8t*)k)[13];
    ((uint_8t*)d)[14] = ((uint_8t*)s)[14] ^ ((uint_8t*)k)[14];
    ((uint_8t*)d)[15] = ((uint_8t*)s)[15] ^ ((uint_8t*)k)[15];
#else
    block16_copy(d, s);
    xor_block(d, k);
#endif
}

static void add_round_key( uint_8t d[N_BLOCK], const uint_8t k[N_BLOCK] )
{
    xor_block(d, k);
}

static void shift_sub_rows( uint_8t st[N_BLOCK] )
{   uint_8t tt;

    st[ 0] = s_box[st[ 0]]; st[ 4] = s_box[st[ 4]];
    st[ 8] = s_box[st[ 8]]; st[12] = s_box[st[12]];

    tt = st[1]; st[ 1] = s_box[st[ 5]]; st[ 5] = s_box[st[ 9]];
    st[ 9] = s_box[st[13]]; st[13] = s_box[ tt ];

    tt = st[2]; st[ 2] = s_box[st[10]]; st[10] = s_box[ tt ];
    tt = st[6]; st[ 6] = s_box[st[14]]; st[14] = s_box[ tt ];

    tt = st[15]; st[15] = s_box[st[11]]; st[11] = s_box[st[ 7]];
    st[ 7] = s_box[st[ 3]]; st[ 3] = s_box[ tt ];
}

static void inv_shift_sub_rows( uint_8t st[N_BLOCK] )
{   uint_8t tt;

    st[ 0] = inv_s_box[st[ 0]]; st[ 4] = inv_s_box[st[ 4]];
    st[ 8] = inv_s_box[st[ 8]]; st[12] = inv_s_box[st[12]];

    tt = st[13]; st[13] = inv_s_box[st[9]]; st[ 9] = inv_s_box[st[5]];
    st[ 5] = inv_s_box[st[1]]; st[ 1] = inv_s_box[ tt ];

    tt = st[2]; st[ 2] = inv_s_box[st[10]]; st[10] = inv_s_box[ tt ];
    tt = st[6]; st[ 6] = inv_s_box[st[14]]; st[14] = inv_s_box[ tt ];

    tt = st[3]; st[ 3] = inv_s_box[st[ 7]]; st[ 7] = inv_s_box[st[11]];
    st[11] = inv_s_box[st[15]]; st[15] = inv_s_box[ tt ];
}

#if defined( VERSION_1 )
  static void mix_sub_columns( uint_8t dt[N_BLOCK] )
  { uint_8t st[N_BLOCK];
    block16_copy(st, dt);
#else
  static void mix_sub_columns( uint_8t dt[N_BLOCK], uint_8t st[N_BLOCK] )
  {
#endif
    dt[ 0] = gfm2_s_box[st[0]] ^ gfm3_s_box[st[5]] ^ s_box[st[10]] ^ s_box[st[15]];
    dt[ 1] = s_box[st[0]] ^ gfm2_s_box[st[5]] ^ gfm3_s_box[st[10]] ^ s_box[st[15]];
    dt[ 2] = s_box[st[0]] ^ s_box[st[5]] ^ gfm2_s_box[st[10]] ^ gfm3_s_box[st[15]];
    dt[ 3] = gfm3_s_box[st[0]] ^ s_box[st[5]] ^ s_box[st[10]] ^ gfm2_s_box[st[15]];

    dt[ 4] = gfm2_s_box[st[4]] ^ gfm3_s_box[st[9]] ^ s_box[st[14]] ^ s_box[st[3]];
    dt[ 5] = s_box[st[4]] ^ gfm2_s_box[st[9]] ^ gfm3_s_box[st[14]] ^ s_box[st[3]];
    dt[ 6] = s_box[st[4]] ^ s_box[st[9]] ^ gfm2_s_box[st[14]] ^ gfm3_s_box[st[3]];
    dt[ 7] = gfm3_s_box[st[4]] ^ s_box[st[9]] ^ s_box[st[14]] ^ gfm2_s_box[st[3]];

    dt[ 8] = gfm2_s_box[st[8]] ^ gfm3_s_box[st[13]] ^ s_box[st[2]] ^ s_box[st[7]];
    dt[ 9] = s_box[st[8]] ^ gfm2_s_box[st[13]] ^ gfm3_s_box[st[2]] ^ s_box[st[7]];
    dt[10] = s_box[st[8]] ^ s_box[st[13]] ^ gfm2_s_box[st[2]] ^ gfm3_s_box[st[7]];
    dt[11] = gfm3_s_box[st[8]] ^ s_box[st[13]] ^ s_box[st[2]] ^ gfm2_s_box[st[7]];

    dt[12] = gfm2_s_box[st[12]] ^ gfm3_s_box[st[1]] ^ s_box[st[6]] ^ s_box[st[11]];
    dt[13] = s_box[st[12]] ^ gfm2_s_box[st[1]] ^ gfm3_s_box[st[6]] ^ s_box[st[11]];
    dt[14] = s_box[st[12]] ^ s_box[st[1]] ^ gfm2_s_box[st[6]] ^ gfm3_s_box[st[11]];
    dt[15] = gfm3_s_box[st[12]] ^ s_box[st[1]] ^ s_box[st[6]] ^ gfm2_s_box[st[11]];
  }

#if defined( VERSION_1 )
  static void inv_mix_sub_columns( uint_8t dt[N_BLOCK] )
  { uint_8t st[N_BLOCK];
    block16_copy(st, dt);
#else
  static void inv_mix_sub_columns( uint_8t dt[N_BLOCK], uint_8t st[N_BLOCK] )
  {
#endif
    dt[ 0] = inv_s_box[gfmul_e[st[ 0]] ^ gfmul_b[st[ 1]] ^ gfmul_d[st[ 2]] ^ gfmul_9[st[ 3]]];
    dt[ 5] = inv_s_box[gfmul_9[st[ 0]] ^ gfmul_e[st[ 1]] ^ gfmul_b[st[ 2]] ^ gfmul_d[st[ 3]]];
    dt[10] = inv_s_box[gfmul_d[st[ 0]] ^ gfmul_9[st[ 1]] ^ gfmul_e[st[ 2]] ^ gfmul_b[st[ 3]]];
    dt[15] = inv_s_box[gfmul_b[st[ 0]] ^ gfmul_d[st[ 1]] ^ gfmul_9[st[ 2]] ^ gfmul_e[st[ 3]]];

    dt[ 4] = inv_s_box[gfmul_e[st[ 4]] ^ gfmul_b[st[ 5]] ^ gfmul_d[st[ 6]] ^ gfmul_9[st[ 7]]];
    dt[ 9] = inv_s_box[gfmul_9[st[ 4]] ^ gfmul_e[st[ 5]] ^ gfmul_b[st[ 6]] ^ gfmul_d[st[ 7]]];
    dt[14] = inv_s_box[gfmul_d[st[ 4]] ^ gfmul_9[st[ 5]] ^ gfmul_e[st[ 6]] ^ gfmul_b[st[ 7]]];
    dt[ 3] = inv_s_box[gfmul_b[st[ 4]] ^ gfmul_d[st[ 5]] ^ gfmul_9[st[ 6]] ^ gfmul_e[st[ 7]]];

    dt[ 8] = inv_s_box[gfmul_e[st[ 8]] ^ gfmul_b[st[ 9]] ^ gfmul_d[st[10]] ^ gfmul_9[st[11]]];
    dt[13] = inv_s_box[gfmul_9[st[ 8]] ^ gfmul_e[st[ 9]] ^ gfmul_b[st[10]] ^ gfmul_d[st[11]]];
    dt[ 2] = inv_s_box[gfmul_d[st[ 8]] ^ gfmul_9[st[ 9]] ^ gfmul_e[st[10]] ^ gfmul_b[st[11]]];
    dt[ 7] = inv_s_box[gfmul_b[st[ 8]] ^ gfmul_d[st[ 9]] ^ gfmul_9[st[10]] ^ gfmul_e[st[11]]];

    dt[12] = inv_s_box[gfmul_e[st[12]] ^ gfmul_b[st[13]] ^ gfmul_d[st[14]] ^ gfmul_9[st[15]]];
    dt[ 1] = inv_s_box[gfmul_9[st[12]] ^ gfmul_e[st[13]] ^ gfmul_b[st[14]] ^ gfmul_d[st[15]]];
    dt[ 6] = inv_s_box[gfmul_d[st[12]] ^ gfmul_9[st[13]] ^ gfmul_e[st[14]] ^ gfmul_b[st[15]]];
    dt[11] = inv_s_box[gfmul_b[st[12]] ^ gfmul_d[st[13]] ^ gfmul_9[st[14]] ^ gfmul_e[st[15]]];
  }

#if defined( AES_ENC_PREKEYED ) || defined( AES_DEC_PREKEYED )

/*  Set the cipher key for the pre-keyed version */

return_type aes_set_key( const unsigned char key[], length_type keylen, aes_context ctx[1] )
{
    uint_8t cc, rc, hi;

    switch( keylen )
    {
    case 16:
    case 128:
        keylen = 16;
        break;
    case 24:
    case 192:
        keylen = 24;
        break;
    case 32:
    case 256:
        keylen = 32;
        break;
    default:
        ctx->rnd = 0;
        return (return_type) -1;
    }
    block_copy(ctx->ksch, key, keylen);
    hi = (keylen + 28) << 2;
    ctx->rnd = (hi >> 4) - 1;
    for( cc = keylen, rc = 1; cc < hi; cc += 4 )
    {   uint_8t tt, t0, t1, t2, t3;

        t0 = ctx->ksch[cc - 4];
        t1 = ctx->ksch[cc - 3];
        t2 = ctx->ksch[cc - 2];
        t3 = ctx->ksch[cc - 1];
        if( cc % keylen == 0 )
        {
            tt = t0;
            t0 = s_box[t1] ^ rc;
            t1 = s_box[t2];
            t2 = s_box[t3];
            t3 = s_box[tt];
            rc = f2(rc);
        }
        else if( keylen > 24 && cc % keylen == 16 )
        {
            t0 = s_box[t0];
            t1 = s_box[t1];
            t2 = s_box[t2];
            t3 = s_box[t3];
        }
        tt = cc - keylen;
        ctx->ksch[cc + 0] = ctx->ksch[tt + 0] ^ t0;
        ctx->ksch[cc + 1] = ctx->ksch[tt + 1] ^ t1;
        ctx->ksch[cc + 2] = ctx->ksch[tt + 2] ^ t2;
        ctx->ksch[cc + 3] = ctx->ksch[tt + 3] ^ t3;
    }
    return 0;
}

#endif

#if defined( AES_ENC_PREKEYED )

/*  Encrypt a single block of 16 bytes */

return_type aes_encrypt( const unsigned char in[N_BLOCK], unsigned char  out[N_BLOCK], const aes_context ctx[1] )
{
    if( ctx->rnd )
    {
        uint_8t s1[N_BLOCK], r;
        copy_and_key( s1, in, ctx->ksch );

        for( r = 1 ; r < ctx->rnd ; ++r )
#if defined( VERSION_1 )
        {
            mix_sub_columns( s1 );
            add_round_key( s1, ctx->ksch + r * N_BLOCK);
        }
#else
        {   uint_8t s2[N_BLOCK];
            mix_sub_columns( s2, s1 );
            copy_and_key( s1, s2, ctx->ksch + r * N_BLOCK);
        }
#endif
        shift_sub_rows( s1 );
        copy_and_key( out, s1, ctx->ksch + r * N_BLOCK );
    }
    else
        return (return_type) -1;
    return 0;
}

#endif

#if defined( AES_DEC_PREKEYED )

/*  Decrypt a single block of 16 bytes */

return_type aes_decrypt( const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK], const aes_context ctx[1] )
{
    if( ctx->rnd )
    {
        uint_8t s1[N_BLOCK], r;
        copy_and_key( s1, in, ctx->ksch + ctx->rnd * N_BLOCK );
        inv_shift_sub_rows( s1 );

        for( r = ctx->rnd ; --r ; )
#if defined( VERSION_1 )
        {
            add_round_key( s1, ctx->ksch + r * N_BLOCK );
            inv_mix_sub_columns( s1 );
        }
#else
        {   uint_8t s2[N_BLOCK];
            copy_and_key( s2, s1, ctx->ksch + r * N_BLOCK );
            inv_mix_sub_columns( s1, s2 );
        }
#endif
        copy_and_key( out, s1, ctx->ksch );
    }
    else
        return (return_type) -1;
    return 0;
}

#endif

#if defined( AES_ENC_128_OTFK )

/*  The 'on the fly' encryption key update for for 128 bit keys */

static void update_encrypt_key_128( uint_8t k[N_BLOCK], uint_8t *rc )
{   uint_8t cc;

    k[0] ^= s_box[k[13]] ^ *rc;
    k[1] ^= s_box[k[14]];
    k[2] ^= s_box[k[15]];
    k[3] ^= s_box[k[12]];
    *rc = f2( *rc );

    for(cc = 4; cc < 16; cc += 4 )
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }
}

/*  Encrypt a single block of 16 bytes with 'on the fly' 128 bit keying */

void aes_encrypt_128( const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK],
                     const unsigned char key[N_BLOCK], unsigned char o_key[N_BLOCK] )
{   uint_8t s1[N_BLOCK], r, rc = 1;

    if(o_key != key)
        block16_copy( o_key, key );
    copy_and_key( s1, in, o_key );

    for( r = 1 ; r < 10 ; ++r )
#if defined( VERSION_1 )
    {
        mix_sub_columns( s1 );
        update_encrypt_key_128( o_key, &rc );
        add_round_key( s1, o_key );
    }
#else
    {   uint_8t s2[N_BLOCK];
        mix_sub_columns( s2, s1 );
        update_encrypt_key_128( o_key, &rc );
        copy_and_key( s1, s2, o_key );
    }
#endif

    shift_sub_rows( s1 );
    update_encrypt_key_128( o_key, &rc );
    copy_and_key( out, s1, o_key );
}

#endif

#if defined( AES_DEC_128_OTFK )

/*  The 'on the fly' decryption key update for for 128 bit keys */

static void update_decrypt_key_128( uint_8t k[N_BLOCK], uint_8t *rc )
{   uint_8t cc;

    for( cc = 12; cc > 0; cc -= 4 )
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }
    *rc = d2(*rc);
    k[0] ^= s_box[k[13]] ^ *rc;
    k[1] ^= s_box[k[14]];
    k[2] ^= s_box[k[15]];
    k[3] ^= s_box[k[12]];
}

/*  Decrypt a single block of 16 bytes with 'on the fly' 128 bit keying */

void aes_decrypt_128( const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK],
                      const unsigned char key[N_BLOCK], unsigned char o_key[N_BLOCK] )
{
    uint_8t s1[N_BLOCK], r, rc = 0x6c;
    if(o_key != key)
        block16_copy( o_key, key );

    copy_and_key( s1, in, o_key );
    inv_shift_sub_rows( s1 );

    for( r = 10 ; --r ; )
#if defined( VERSION_1 )
    {
        update_decrypt_key_128( o_key, &rc );
        add_round_key( s1, o_key );
        inv_mix_sub_columns( s1 );
    }
#else
    {   uint_8t s2[N_BLOCK];
        update_decrypt_key_128( o_key, &rc );
        copy_and_key( s2, s1, o_key );
        inv_mix_sub_columns( s1, s2 );
    }
#endif
    update_decrypt_key_128( o_key, &rc );
    copy_and_key( out, s1, o_key );
}

#endif

#if defined( AES_ENC_256_OTFK )

/*  The 'on the fly' encryption key update for for 256 bit keys */

static void update_encrypt_key_256( uint_8t k[2 * N_BLOCK], uint_8t *rc )
{   uint_8t cc;

    k[0] ^= s_box[k[29]] ^ *rc;
    k[1] ^= s_box[k[30]];
    k[2] ^= s_box[k[31]];
    k[3] ^= s_box[k[28]];
    *rc = f2( *rc );

    for(cc = 4; cc < 16; cc += 4)
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }

    k[16] ^= s_box[k[12]];
    k[17] ^= s_box[k[13]];
    k[18] ^= s_box[k[14]];
    k[19] ^= s_box[k[15]];

    for( cc = 20; cc < 32; cc += 4 )
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }
}

/*  Encrypt a single block of 16 bytes with 'on the fly' 256 bit keying */

void aes_encrypt_256( const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK],
                      const unsigned char key[2 * N_BLOCK], unsigned char o_key[2 * N_BLOCK] )
{
    uint_8t s1[N_BLOCK], r, rc = 1;
    if(o_key != key)
    {
        block16_copy( o_key, key );
        block16_copy( o_key + 16, key + 16 );
    }
    copy_and_key( s1, in, o_key );

    for( r = 1 ; r < 14 ; ++r )
#if defined( VERSION_1 )
    {
        mix_sub_columns(s1);
        if( r & 1 )
            add_round_key( s1, o_key + 16 );
        else
        {
            update_encrypt_key_256( o_key, &rc );
            add_round_key( s1, o_key );
        }
    }
#else
    {   uint_8t s2[N_BLOCK];
        mix_sub_columns( s2, s1 );
        if( r & 1 )
            copy_and_key( s1, s2, o_key + 16 );
        else
        {
            update_encrypt_key_256( o_key, &rc );
            copy_and_key( s1, s2, o_key );
        }
    }
#endif

    shift_sub_rows( s1 );
    update_encrypt_key_256( o_key, &rc );
    copy_and_key( out, s1, o_key );
}

#endif

#if defined( AES_DEC_256_OTFK )

/*  The 'on the fly' encryption key update for for 256 bit keys */

static void update_decrypt_key_256( uint_8t k[2 * N_BLOCK], uint_8t *rc )
{   uint_8t cc;

    for(cc = 28; cc > 16; cc -= 4)
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }

    k[16] ^= s_box[k[12]];
    k[17] ^= s_box[k[13]];
    k[18] ^= s_box[k[14]];
    k[19] ^= s_box[k[15]];

    for(cc = 12; cc > 0; cc -= 4)
    {
        k[cc + 0] ^= k[cc - 4];
        k[cc + 1] ^= k[cc - 3];
        k[cc + 2] ^= k[cc - 2];
        k[cc + 3] ^= k[cc - 1];
    }

    *rc = d2(*rc);
    k[0] ^= s_box[k[29]] ^ *rc;
    k[1] ^= s_box[k[30]];
    k[2] ^= s_box[k[31]];
    k[3] ^= s_box[k[28]];
}

/*  Decrypt a single block of 16 bytes with 'on the fly'
    256 bit keying
*/
void aes_decrypt_256( const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK],
                      const unsigned char key[2 * N_BLOCK], unsigned char o_key[2 * N_BLOCK] )
{
    uint_8t s1[N_BLOCK], r, rc = 0x80;

    if(o_key != key)
    {
        block16_copy( o_key, key );
        block16_copy( o_key + 16, key + 16 );
    }

    copy_and_key( s1, in, o_key );
    inv_shift_sub_rows( s1 );

    for( r = 14 ; --r ; )
#if defined( VERSION_1 )
    {
        if( ( r & 1 ) )
        {
            update_decrypt_key_256( o_key, &rc );
            add_round_key( s1, o_key + 16 );
        }
        else
            add_round_key( s1, o_key );
        inv_mix_sub_columns( s1 );
    }
#else
    {   uint_8t s2[N_BLOCK];
        if( ( r & 1 ) )
        {
            update_decrypt_key_256( o_key, &rc );
            copy_and_key( s2, s1, o_key + 16 );
        }
        else
            copy_and_key( s2, s1, o_key );
        inv_mix_sub_columns( s1, s2 );
    }
#endif
    copy_and_key( out, s1, o_key );
}

#endif
