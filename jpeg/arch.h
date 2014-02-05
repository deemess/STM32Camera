#ifndef __ARCH_H__
#define __ARCH_H__

#ifdef __cplusplus
//extern "C" {
#endif

typedef unsigned char color;

#ifdef _MSC_VER
#	include <intrin.h>
#	include <emmintrin.h>
#	include <io.h>
#	define CACHE_ALIGN __declspec(align(32))
#else
#	include <stdint.h>
#	include <unistd.h>
#	define CACHE_ALIGN __attribute__ ((aligned(32)))
#	define O_BINARY	0
#endif

#ifdef __cplusplus
//}
#endif

#endif//__ARCH_H__
