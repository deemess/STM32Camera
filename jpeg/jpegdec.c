#include <stdlib.h>
#include <stdio.h>
//#include <memory.h>

#include "jpegdec.h"

#define CACHE_ALIGN __declspec(align(32))

typedef struct bitbuffer_s
{
	int      buf;
	unsigned n;
}
bitbuffer_t;

static bitbuffer_t bitbuf;


static FILE *jpeg_file;

static unsigned char readbyte(void)
{
	return fgetc(jpeg_file);
}

static void readbytes(unsigned char *ptr, int len)
{
	while (len > 0) len--, *ptr++ = readbyte();
}

static unsigned short readword(void)
{
	unsigned short w = readbyte();

	return (w << 8) | readbyte();
}

/******************************************************************************
**  getbit
**  --------------------------------------------------------------------------
**  Get bit from bit-buffer. Does not change bit-buffer data.
**  
**  ARGUMENTS:
**      pbb     - pointer to bit-buffer context;
**
**  RETURN: the bit
******************************************************************************/
static unsigned getbit(const bitbuffer_t *pbb)
{
	return pbb->buf < 0;
}

static unsigned char getbyte(bitbuffer_t *const pbb)
{
	if (pbb->n < 8)
	{
		int b = readbyte();
		pbb->buf |= b << (24 - pbb->n);
		pbb->n += 8;

		if (b == 0xFF) {
			// marker processing
		}
	}

	return (unsigned)pbb->buf >> 24;
}

static unsigned short getword(bitbuffer_t *const pbb)
{
	while (pbb->n < 16)
	{
		int b = readbyte();
		pbb->buf |= b << (24 - pbb->n);
		pbb->n += 8;

		if (b == 0xFF) {
			// marker processing
		}
	}

	return (unsigned)pbb->buf >> 16;
}

/******************************************************************************
**  nextbit
**  --------------------------------------------------------------------------
**  Discard current bit in bit-buffer and fetch the next bit.
**  
**  ARGUMENTS:
**      pbb     - pointer to bit-buffer context;
**
**  RETURN: -
******************************************************************************/
static void nextbit(bitbuffer_t *const pbb)
{
	if (pbb->n) {
		pbb->n--;
		pbb->buf <<= 1;
	}

	if (!pbb->n)
	{
		unsigned b = readbyte();

		if (b == 0xFF) {
			unsigned b2 = readbyte();

			if (b2) {
				// marker
				return;
			}
		}

		pbb->buf = b << 24;
		pbb->n = 8;
	}
}

// zig-zag table
const unsigned char zig[64] =
{
	 0,
	 1, 8,
	16, 9, 2, 
	 3,10,17,24,
	32,25,18,11, 4,
	 5,12,19,26,33,40,
	48,41,34,27,20,13, 6,
	 7,14,21,28,35,42,49,56,
	57,50,43,36,29,22,15,
	23,30,37,44,51,58,
	59,52,45,38,31,
	39,46,53,60,
	61,54,47,
	55,62,
	63
};


struct htree_node_s
{
	int      code;
	unsigned child[2];
};

typedef struct htree_node_s htree_node_t;

struct htree_s
{
	unsigned     size;
	htree_node_t tree[1];
};

typedef struct htree_s htree_t;


/******************************************************************************
**  huffman_tree_size
**  --------------------------------------------------------------------------
**  Calculates the number of nodes for a Huffman tree.
**  
**  ARGUMENTS:
**      ncodes   - array containing number of codes on each level;
**      nmax     - number of elements in the array;
**
**  RETURN: Number of nodes.
******************************************************************************/
static unsigned huffman_tree_size(const unsigned char *ncodes, const unsigned nmax)
{
	unsigned n;
	unsigned i;
	unsigned skip;

	for (n = 1, i = 0, skip = 0; i < nmax; i++)
	{
		n += (1 << (i+1)) - skip;
		skip = (skip + ncodes[i])*2;
	}

	return n;
}

/******************************************************************************
**  huffman_tree_create
**  --------------------------------------------------------------------------
**  Allocate memory for the Huffman tree and build it.
**  
**  
**  
**  ARGUMENTS:
**      ncodes   - array containing number of codes on each level;
**      nmax     - number of elements in the array;
**      codes    - array of codes;
**
**  RETURN: Huffman tree ptr., NULL - error.
******************************************************************************/
htree_t* huffman_tree_create(const unsigned char ncodes[],
							 const unsigned nmax,
							 const unsigned char codes[])
{
	htree_t      *htree;
	htree_node_t *tree;
	unsigned     skip = 0;
	unsigned     free = 0;
	unsigned     curr = free++;
	unsigned     i, j, k;
	unsigned     cmax;

	// calculate number of codes
	for (cmax = 0, i = 0; i < nmax; i++) cmax += ncodes[i];

	i = huffman_tree_size(ncodes, nmax);

	htree = malloc(sizeof(htree_t) + (i-1)*sizeof(htree_node_t));

	if (!htree) return NULL;

	tree = htree->tree;

	tree[curr].code = -1;
	tree[curr].child[0] = free++;
	tree[curr].child[1] = free++;
	curr++;

	for (k = 0, i = 0; i < nmax && k < cmax; i++)
	{
		unsigned jmax = (1 << (i+1)) - skip;

		for(j = 0; j < ncodes[i]; j++)
		{
			tree[curr++].code = codes[k++];
		}

		for(; j < jmax; j++)
		{
			tree[curr].code = -1;
			tree[curr].child[0] = free++;
			tree[curr].child[1] = free++;
			curr++;
		}

		skip += ncodes[i];
		skip *= 2;
	}

	htree->size = free;

	return htree;
}

void huffman_tree_destroy(htree_t *htree)
{
	free(htree);
}

/******************************************************************************
**  huffman_decode
**  --------------------------------------------------------------------------
**  Decode bitstream using previously constructed Huffman tree.
**  I function returns nonzero value (number of bits) than it has found
**  a valid code which is returned via 'code' parameter.
**  
**  ARGUMENTS:
**      htree     - Huffman tree ptr.;
**      bitstream - input bitstream;
**      code      - ptr. to decoded code;
**
**  RETURN: The number of processed bits in bitstream.
******************************************************************************/
unsigned huffman_decode(const htree_t *htree, int *code)
{
	unsigned n = 0;
	unsigned curr = 0;

	while (curr < htree->size)
	{
		unsigned bit = getbit(&bitbuf);

		nextbit(&bitbuf);

		curr = htree->tree[curr].child[bit];
		*code = htree->tree[curr].code;
		n++;

		if (*code >= 0)
			return n;
	}

	return 0;
}

/******************************************************************************
**  read_vli
**  --------------------------------------------------------------------------
**  Read JPEG variable length integer (VLI) from bitstream.
**  
**  ARGUMENTS:
**      size  - VLI length in bits;
**
**  RETURN: VLI number
******************************************************************************/
unsigned read_vli(unsigned size)
{
	unsigned bits = 0;

	while (size)
	{
		size--;
		bits = (bits << 1) | getbit(&bitbuf);
		nextbit(&bitbuf);
	}

	return bits;
}


/******************************************************************************
**  vli2int
**  --------------------------------------------------------------------------
**  Convert JPEG variable length integer (VLI) into integer number.
**  VLI is called "Baseline Entropy Coding Symbol-2" in the standard.
**  
**  ARGUMENTS:
**      vli       - JPEG variable length integer;
**      size      - size of VLI in bits;
**
**  RETURN: integer number.
******************************************************************************/
static int vli2int(const unsigned vli, const unsigned size)
{
	if (!size) return 0;

	if ((vli << 1) < (1 << size))
		return vli + 1 - (1 << size);

	return vli;
}


#define SIZEA(_array_)	(sizeof(_array_)/sizeof(_array_[0]))

#define MARKER			0xFF
#define MARKER_SOF		0xC0
#define MARKER_DHT		0xC4
#define MARKER_SOI		0xD8
#define MARKER_EOI		0xD9
#define MARKER_SOS		0xDA
#define MARKER_DQT		0xDB
#define MARKER_APP0		0xE0

typedef struct jpeg_app0_s
{
	unsigned char  id[5];
	unsigned short version;
	unsigned char  xyunits;
	unsigned short xdensity;
	unsigned short ydensity;
	unsigned char  thumbnwidth;
	unsigned char  thumbnheight;
}
jpeg_app0_t;

typedef struct jpeg_sofcomp_s
{
	unsigned char  id;			// component identifier
	unsigned char  hvf;			// horizontal,vertical sampling factor
	unsigned char  qt;			// quantization table destination selector
}
jpeg_sofcomp_t;

typedef struct jpeg_sof_s
{
	unsigned char  precision;	// sample precision
	unsigned short height;		// image height
	unsigned short width;		// image width
	unsigned char  ncomp;		// number of image components in frame
	jpeg_sofcomp_t comp[3];		// image components
}
jpeg_sof_t;

typedef struct jpeg_soscomp_s
{
	unsigned char  id;			// component identifier
	unsigned char  ht;			// DC,AC huffman table destination selector
}
jpeg_soscomp_t;

typedef struct jpeg_sos_s
{
	unsigned char  ncomp;		// number of image components in scan
	jpeg_soscomp_t comp[3];		// image components
	unsigned char  ss;			// start of spectral or predictor selection
	unsigned char  se;			// end of spectral selection
	unsigned char  bf;			// successive approximation bit position high,low
}
jpeg_sos_t;

typedef struct jpeg_dqt_s
{
	unsigned char  ntbl;		// number of tables
	unsigned char  id[2];		// quantization table precision, destination identifier
	unsigned char  qtable[2][64];
}
jpeg_dqt_t;

typedef struct jpeg_dhtt_s
{
	unsigned char  dc_nrcodes[16];
	unsigned char  ac_nrcodes[16];
	unsigned char  dc_values[32];
	unsigned char  ac_values[192];
	htree_t        *htree_dc;	// DC huffman tree
	htree_t        *htree_ac;	// AC huffman tree
}
jpeg_dhtt_t;

typedef struct jpeg_dht_s
{
	unsigned char  ntbl;		// number of huffman tables
	unsigned char  id[4];		// huffman table class (AC/DC), destination identifier
	jpeg_dhtt_t    tbl[2];		// huffman tables
}
jpeg_dht_t;


typedef struct jpeg_ctx_s
{
	htree_t       *htree_dc;	// DC huffman tree
	htree_t       *htree_ac;	// AC huffman tree
	unsigned char *qtable;		// quantization table
	short         dc;			// DC coef
}
jpeg_ctx_t;

typedef struct jpeg_dec_s
{
	unsigned      state;

	jpeg_ctx_t    ctx[3];	// decoding contexts
	// headers
	jpeg_sof_t    sof;		// start of frame header
	jpeg_sos_t    sos;		// start of scan header
	jpeg_app0_t   app0;		// app0 (jfif) header
	jpeg_dqt_t    dqt;		// quantization tables header
	jpeg_dht_t    dht;		// huffman tables header
}
jpeg_dec_t;


jpeg_dec_t* jpegdec_create(char *filename)
{
	jpeg_dec_t *dec;

	dec = malloc(sizeof(*dec));

	if (dec) {
		memset(dec, 0, sizeof(*dec));

		jpeg_file = fopen(filename, "rb");
	}

	return dec;
}

void jpegdec_destroy(jpeg_dec_t *const dec)
{
	unsigned i;

	for (i = 0; i < 3; i++)
	{
		if (dec->ctx[i].htree_ac) huffman_tree_destroy(dec->ctx[i].htree_ac);
		if (dec->ctx[i].htree_dc) huffman_tree_destroy(dec->ctx[i].htree_dc);
	}
}


static void skip_bytes(int len)
{
	while (len-->0) readbyte();
}

static void read_marker_app0(jpeg_app0_t *const app0)
{
	const int len = readword()-2;

	if (len == 14)
	{
		readbytes(app0->id, sizeof(app0->id));

		app0->version      = readword();
		app0->xyunits      = readbyte();
		app0->xdensity     = readword();
		app0->ydensity     = readword();
		app0->thumbnwidth  = readbyte();
		app0->thumbnheight = readbyte();
	}
	else skip_bytes(len);
}

static void read_marker_sof(jpeg_sof_t *const sof)
{
	const int len = readword()-2;

	if (len == 15)
	{
		unsigned i;

		sof->precision = readbyte();
		sof->height    = readword();
		sof->width     = readword();
		sof->ncomp     = readbyte();

		for (i = 0; i < sof->ncomp; i++)
		{
			sof->comp[i].id  = readbyte();
			sof->comp[i].hvf = readbyte();
			sof->comp[i].qt  = readbyte();
		}
	}
	else skip_bytes(len);
}

static void read_marker_sos(jpeg_sos_t *const sos)
{
	int len = readword()-2;

	if (len == 10)
	{
		unsigned i;

		sos->ncomp = readbyte();

		for (i = 0; i < sos->ncomp; i++)
		{
			sos->comp[i].id = readbyte();
			sos->comp[i].ht = readbyte();
		}

		sos->ss = readbyte();
		sos->se = readbyte();
		sos->bf = readbyte();
	}
	else skip_bytes(len);
}

static void read_marker_dht(jpeg_dht_t *const dht)
{
	unsigned n = 0;
	int      len = readword()-2;
	
	while (len > 0)
	{
		unsigned id;

		dht->id[n] = readbyte();
		id = dht->id[n] & 15; // Y, Cb or Cr ?

		len -= 1;

		if (id < 2)
		{
			if (dht->id[n] >> 4) // AC or DC ?
			{
				unsigned sum, i;

				if ((len - 16) < 0) break;
				len -= 16;

				for (sum = 0, i = 0; i < 16; i++)
					sum += dht->tbl[id].ac_nrcodes[i] = readbyte();

				if ((len - sum) < 0) break;
				len -= sum;

				readbytes(dht->tbl[id].ac_values,  sum);
			}
			else {
				unsigned sum, i;

				if ((len - 16) < 0) break;
				len -= 16;

				for (sum = 0, i = 0; i < 16; i++)
					sum += dht->tbl[id].dc_nrcodes[i] = readbyte();

				if ((len - sum) < 0) break;
				len -= sum;

				readbytes(dht->tbl[id].dc_values,  sum);
			}
		}
		else break;

		n++;
	}

	dht->ntbl = n;

	// skip the rest of data
	skip_bytes(len);
}

static void read_marker_dqt(jpeg_dqt_t *const dqt)
{
	unsigned n = 0;
	int      len = readword()-2;
	
	while (len > 0)
	{
		unsigned id = dqt->id[n] = readbyte(); // Cb/Cr or Y ?

		len -= 1;

		if (id < 2)
		{
			if ((len - 64) < 0) break;

			len -= 64;

			readbytes(dqt->qtable[id], 64);
		}
		else break;

		n++;
	}

	dqt->ntbl = n;

	// skip the rest of data
	skip_bytes(len);
}


static void jpegdec_build_ht(jpeg_dec_t *const dec)
{
	unsigned i;

	for (i = 0; i < dec->dht.ntbl; i++)
	{
		unsigned id = dec->dht.id[i] & 15;

		if (dec->dht.id[i] >> 4) // AC or DC ?
		{
			dec->dht.tbl[id].htree_ac = huffman_tree_create(
				dec->dht.tbl[id].ac_nrcodes, 16,
				dec->dht.tbl[id].ac_values);

			if (!dec->dht.tbl[id].htree_ac)
				printf("Error: cannot create AC Huffman tree %d\n", id);
		}
		else {
			dec->dht.tbl[id].htree_dc = huffman_tree_create(
				dec->dht.tbl[id].dc_nrcodes, 16,
				dec->dht.tbl[id].dc_values);

			if (!dec->dht.tbl[id].htree_dc)
				printf("Error: cannot create DC Huffman tree %d\n", id);
		}
	}
}

static void jpegdec_select_ht(jpeg_dec_t *const dec)
{
	unsigned i;

	for (i = 0; i < dec->sos.ncomp; i++)
	{
		unsigned id;

		id = dec->sos.comp[i].ht & 15; // AC selector

		dec->ctx[i].htree_ac = dec->dht.tbl[id].htree_ac;

		if (!dec->ctx[i].htree_ac)
			printf("Error: %d component cannot select AC Huffman tree %d\n", i, id);

		id = dec->sos.comp[i].ht >> 4; // DC selector

		dec->ctx[i].htree_dc = dec->dht.tbl[id].htree_dc;

		if (!dec->ctx[i].htree_dc)
			printf("Error: %d component cannot select DC Huffman tree %d\n", i, id);
	}
}

static void jpegdec_select_qt(jpeg_dec_t *const dec)
{
	unsigned i;

	for (i = 0; i < dec->sof.ncomp; i++)
	{
		unsigned id = dec->sof.comp[i].qt;

		dec->ctx[i].qtable = dec->dqt.qtable[id];

		if (!dec->ctx[i].qtable)
			printf("Error: %d component cannot select Q table %d\n", i, id);
	}
}

static void jpegdec_read_marker(jpeg_dec_t *const dec, const unsigned id)
{
	switch (id)
	{
	case MARKER_SOS:
		read_marker_sos(&dec->sos);
		jpegdec_select_ht(dec);
		break;

	case MARKER_SOF:
		read_marker_sof(&dec->sof);
		jpegdec_select_qt(dec);
		break;

	case MARKER_DQT:
		read_marker_dqt(&dec->dqt);
		break;

	case MARKER_DHT:
		read_marker_dht(&dec->dht);
		jpegdec_build_ht(dec);
		break;

	case MARKER_APP0:
		read_marker_app0(&dec->app0);
		break;

	default:
		printf("Error: unknown marker %02x\n", id);
		break;
	}
}

unsigned find_start_of_image(void)
{
	unsigned n = 0;

	do {
		while (readbyte() != MARKER) n++;
		n += 2;
	}
	while (readbyte() != MARKER_SOI);
	
	return n;
}

int jpegdec_read_headers(jpeg_dec_t *const dec)
{
	find_start_of_image();
	
	while(readbyte() == MARKER)
	{
		unsigned id = readbyte();

		jpegdec_read_marker(dec, id);

		if (MARKER_SOS == id) {
			nextbit(&bitbuf);
			return 1;
		}
	}

	return 0;
}

void jpegdec_get_info(const jpeg_dec_t *dec, jpeg_info_t *const info)
{
	info->height = dec->sof.height;
	info->width  = dec->sof.width;
}

int jpegdec_decode(jpeg_dec_t *const dec, unsigned id, short pixels[64])
{
	jpeg_ctx_t *const ctx = &dec->ctx[id];
	unsigned   n, i;
	unsigned   code = 0;
	unsigned   vli;
	unsigned   vlilen;
	int        dc;

	n = huffman_decode(ctx->htree_dc, &vlilen);
	vli = read_vli(vlilen);
	dc = vli2int(vli, vlilen);

	// calculate and save DC
	pixels[0] = ctx->dc = ctx->dc + dc;

	for (i = 1; i < 64;)
	{
		unsigned code;
		unsigned zrl;
		int      ac;

		n = huffman_decode(ctx->htree_ac, &code);
		zrl = code >> 4;
		vlilen = code & 15;

		if (!vlilen) {
			if (zrl == 15) zrl++;
			else if (zrl == 0) zrl = 64 - i;
		}

		while (zrl) {
			zrl--;
			pixels[zig[i++]] = 0;
		}

		if (vlilen) {
			vli = read_vli(vlilen);
			ac = vli2int(vli, vlilen);
			pixels[zig[i++]] = ac;
		}
	}

	for (i = 0; i < 64; i++)
	{
		pixels[zig[i]] *= ctx->qtable[i];
	}

	return 0;
}
