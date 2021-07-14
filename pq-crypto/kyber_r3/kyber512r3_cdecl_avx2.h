#pragma once

/* The C ABI on MacOS exports all symbols with a leading
 * underscore. This means that any symbols we refer to from
 * C files (functions) can't be found, and all symbols we
 * refer to from ASM also can't be found.
 *
 * This define helps us get around this
 */
#ifdef __ASSEMBLER__
#if defined(__WIN32__) || defined(__APPLE__)
#define decorate(s) ##s
#define cdecl2(s) decorate(s)
#define cdecl(s) cdecl2(S2N_KYBER_512_R3_NAMESPACE(##s))
#else
#define cdecl(s) S2N_KYBER_512_R3_NAMESPACE(##s)
#endif
#endif
