#pragma once

#include "kyber512r3_params.h"

#ifdef S2N_KYBER512R3_AVX2_BMI2

#define _16XQ            0
#define _16XQINV        16
#define _16XV           32
#define _16XFLO         48
#define _16XFHI         64
#define _16XMONTSQLO    80
#define _16XMONTSQHI    96
#define _16XMASK       112
#define _REVIDXB       128
#define _REVIDXD       144
#define _ZETAS_EXP     160
#define _16XSHIFT      624
#include <stdio.h>
#include <stdlib.h>

/* The C ABI on MacOS exports all symbols with a leading
 * underscore. This means that any symbols we refer to from
 * C files (functions) can't be found, and all symbols we
 * refer to from ASM also can't be found.
 *
 * This define helps us get around this
 */
#define _cdecl(s) _##s
#define cdecl(s) s

#ifndef __ASSEMBLER__
#include "kyber512r3_align_avx2.h"
typedef ALIGNED_INT16(640) qdata_t;
#define qdata S2N_KYBER_512_R3_NAMESPACE(qdata)
extern const qdata_t qdata;
#endif

#endif
