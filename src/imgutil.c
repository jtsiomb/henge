#include <math.h>
#include "imgutil.h"

#define CLAMP(x, a, b)	((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#define ASHIFT	24
#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0

#define PACK_COLOR32(a,r,g,b) \
	((((a) & 0xff) << ASHIFT) | \
	 (((r) & 0xff) << RSHIFT) | \
	 (((g) & 0xff) << GSHIFT) | \
	 (((b) & 0xff) << BSHIFT))

#define UNP_RED32(p) \
	(((p) >> RSHIFT) & 0xff)

#define UNP_GREEN32(p) \
	(((p) >> GSHIFT) & 0xff)

#define UNP_BLUE32(p) \
	(((p) >> BSHIFT) & 0xff)

#define UNP_ALPHA32(p) \
	(((p) >> ASHIFT) & 0xff)


void conv_32bpp_to_float(uint32_t *src, float *dst, int xsz, int ysz)
{
	int i, sz = xsz * ysz;
	for(i=0; i<sz; i++) {
		dst[i * 4] = UNP_RED32(src[i]) / 255.0f;
		dst[i * 4 + 1] = UNP_GREEN32(src[i]) / 255.0f;
		dst[i * 4 + 2] = UNP_BLUE32(src[i]) / 255.0f;
		dst[i * 4 + 3] = UNP_ALPHA32(src[i]) / 255.0f;
	}
}

void conv_float_to_32bpp(float *src, uint32_t *dst, int xsz, int ysz)
{
	int i, sz = xsz * ysz;
	for(i=0; i<sz; i++) {
		int r = src[i * 4] * 255.0f;
		int g = src[i * 4 + 1] * 255.0f;
		int b = src[i * 4 + 2] * 255.0f;
		int a = src[i * 4 + 3] * 255.0f;
		dst[i] = PACK_COLOR32(CLAMP(a, 0, 255), CLAMP(r, 0, 255), CLAMP(g, 0, 255), CLAMP(b, 0, 255));
	}
}
