#ifndef __JPEGDEC_H__
#define __JPEGDEC_H__

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char zig[64];

struct jpeg_dec_s;
typedef struct jpeg_dec_s jpeg_dec_t;

typedef struct jpeg_info_s
{
	unsigned short height;
	unsigned short width;
}
jpeg_info_t;

jpeg_dec_t* jpegdec_create(char *filename);
void jpegdec_destroy(jpeg_dec_t *const dec);
int jpegdec_read_headers(jpeg_dec_t *const dec);
void jpegdec_get_info(const jpeg_dec_t *dec, jpeg_info_t *const info);
int jpegdec_decode(jpeg_dec_t *const dec, unsigned id, short pixels[64]);

#ifdef __cplusplus
}
#endif

#endif//__JPEGDEC_H__
