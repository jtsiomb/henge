#ifndef IMGUTIL_H_
#define IMGUTIL_H_

#include "int_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void conv_32bpp_to_float(uint32_t *src, float *dst, int xsz, int ysz);
void conv_float_to_32bpp(float *src, uint32_t *dst, int xsz, int ysz);

#ifdef __cplusplus
}
#endif

#endif	/* IMGUTIL_H_ */
