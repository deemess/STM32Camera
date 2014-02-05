#ifndef __DCT_H__
#define __DCT_H__

#ifdef __cplusplus
extern "C" {
#endif

// integer DCTs
void dct(short pixel[8][8], short data[8][8]);

#ifdef __cplusplus
}
#endif

#endif//__DCT_H__
