#ifndef __IDCT_H__
#define __IDCT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_ALIGN __declspec(align(32))

void idct(short data[8][8], short pixel[8][8]);

#ifdef __cplusplus
}
#endif

#endif//__IDCT_H__
