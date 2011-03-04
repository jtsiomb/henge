#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "int_types.h"
#include "pixmap.h"
#include "imago2.h"
#include "imgutil.h"
#include "errlog.h"

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

using namespace henge;

static int pixel_size(PixmapFormat fmt);

Pixmap::Pixmap()
{
	manage_pixels = false;
	pixels = 0;
	xsz = ysz = 0;
	fmt = PIX_RGBA32;
	bytes_per_pixel = 4;
}

Pixmap::~Pixmap()
{
	if(manage_pixels) {
		free(pixels);
	}
}

bool Pixmap::set_pixels(int xsz, int ysz, PixmapFormat fmt, void *pixels)
{
	void *tmp;
	if(!(tmp = malloc(xsz * ysz * pixel_size(fmt)))) {
		return false;
	}

	if(manage_pixels) {
		free(this->pixels);
	}
	this->pixels = tmp;
	this->xsz = xsz;
	this->ysz = ysz;
	this->fmt = fmt;
	bytes_per_pixel = pixel_size(fmt);
	manage_pixels = true;

	if(pixels) {
		memcpy(this->pixels, pixels, xsz * ysz * bytes_per_pixel);
	}

	return true;
}

bool Pixmap::load(const char *name)
{
	void *img;
	int img_xsz, img_ysz;

	enum img_fmt ifmt = this->fmt == PIX_FLOAT ? IMG_FMT_RGBAF : IMG_FMT_RGBA32;
	if(!(img = (void*)img_load_pixels(name, &img_xsz, &img_ysz, ifmt))) {
		return false;
	}

	if(manage_pixels) {
		free(pixels);
	}
	pixels = img;
	xsz = img_xsz;
	ysz = img_ysz;
	bytes_per_pixel = pixel_size(fmt);
	manage_pixels = true;

	return true;
}

bool Pixmap::save(const char *name)
{
	return false; // TODO
}

bool Pixmap::generate(const char *expr)
{
	return false;	// TODO
}

bool Pixmap::is_transparent() const
{
	int i, j;
	float *fpix = 0;
	uint32_t *ipix = 0;

	if(!pixels) return false;

	if(fmt == PIX_RGBA32) {
		ipix = (uint32_t*)pixels;
	} else {
		fpix = (float*)pixels;
	}

	for(i=0; i<xsz; i++) {
		for(j=0; j<ysz; j++) {
			if(ipix) {
				if((*ipix >> 3) < 0xff) {
					return true;
				}
				ipix++;
			} else {
				if(fpix[3] > 1.0) {
					return true;
				}
				fpix += 4;
			}
		}
	}
	return false;
}

#define CONVERSION(a, b)	(((int)(a) << 8) | (int)(b))

void henge::convert_pixmap(Pixmap *pix, PixmapFormat tofmt)
{
	float *fpix;
	uint32_t *ipix;

	if(pix->fmt == tofmt) {
		return;
	}

	switch(CONVERSION(pix->fmt, tofmt)) {
	case CONVERSION(PIX_RGBA32, PIX_FLOAT):
		if(!(fpix = (float*)malloc(pix->xsz * pix->ysz * 4 * sizeof *fpix))) {
			error("convert_pixmap: %s\n", strerror(errno));
			return;
		}
		conv_32bpp_to_float((uint32_t*)pix->pixels, fpix, pix->xsz, pix->ysz);
		if(pix->manage_pixels) {
			free(pix->pixels);
		}
		pix->pixels = fpix;
		pix->fmt = PIX_FLOAT;
		pix->bytes_per_pixel = sizeof *fpix * 4;
		pix->manage_pixels = true;
		break;

	case CONVERSION(PIX_FLOAT, PIX_RGBA32):
		if(!(ipix = (uint32_t*)malloc(pix->xsz * pix->ysz * sizeof *ipix))) {
			error("convert_pixmap: %s\n", strerror(errno));
			return;
		}
		conv_float_to_32bpp((float*)pix->pixels, ipix, pix->xsz, pix->ysz);
		if(pix->manage_pixels) {
			free(pix->pixels);
		}
		pix->pixels = ipix;
		pix->fmt = PIX_RGBA32;
		pix->bytes_per_pixel = sizeof *ipix;
		pix->manage_pixels = true;
		break;

	default:
		error("convert_pixmap: unsupported conversion requested\n");
		break;
	}
}

void henge::copy_pixels(Pixmap *dest, Pixmap *src)
{
	copy_pixels(dest, 0, 0, src, 0, 0, src->xsz, src->ysz);
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))

// TODO make negative destx/desty and sx/sy work too
void henge::copy_pixels(Pixmap *dest, int destx, int desty, Pixmap *src, int sx, int sy, int width, int height)
{
	if(dest->fmt != src->fmt) {
		error("can't copy_pixels between pixmaps with different formats\n");
		return;
	}

	int pixsz = dest->bytes_per_pixel;

	info("copy_pixels (%d %d) -> (%d %d) [%d %d]\n", sx, sy, destx, desty, width, height);

	unsigned char *sptr = (unsigned char*)src->pixels + (sy * src->xsz + sx) * pixsz;
	unsigned char *dptr = (unsigned char*)dest->pixels + (destx * dest->xsz + desty) * pixsz;

	int span = MIN(width, MIN(src->xsz - sx, dest->xsz - destx));
	height = MIN(height, MIN(src->ysz - sy, dest->ysz - desty));

	if(span <= 0 || height <= 0) {
		return;
	}

	for(int i=0; i<height; i++) {
		memcpy(dptr, sptr, span * pixsz);
		dptr += dest->xsz * pixsz;
		sptr += src->xsz * pixsz;
	}
}

void henge::rotate_pixmap(Pixmap *pix, int deg)
{
	switch(deg) {
	case 180:
		mirror_pixmap(pix);
		flip_pixmap(pix);
		break;

	default:
		warning("rotate_pixmap(%d) unimplemented\n", deg);
		break;
	}
}

void henge::mirror_pixmap(Pixmap *pix)
{
	unsigned char *start, *front, *back;

	start = (unsigned char*)pix->pixels;
	for(int i=0; i<pix->ysz; i++) {

		front = start;
		start += pix->xsz * pix->bytes_per_pixel;
		back = start - pix->bytes_per_pixel;

		for(int j=0; j<pix->xsz / 2; j++) {
			for(int k=0; k<pix->bytes_per_pixel; k++) {
				unsigned char tmp = front[k];
				front[k] = back[k];
				back[k] = tmp;
			}
			front += pix->bytes_per_pixel;
			back -= pix->bytes_per_pixel;
		}
	}
}

void henge::flip_pixmap(Pixmap *pix)
{
	void *buf = alloca(pix->xsz * pix->bytes_per_pixel);

	int scansz = pix->xsz * pix->bytes_per_pixel;
	unsigned char *top = (unsigned char*)pix->pixels;
	unsigned char *bottom = top + (pix->ysz - 1) * scansz;

	for(int i=0; i<pix->ysz / 2; i++) {
		memcpy(buf, top, scansz);
		memcpy(top, bottom, scansz);
		memcpy(bottom, buf, scansz);

		top += scansz;
		bottom -= scansz;
	}
}

static int pixel_size(PixmapFormat fmt)
{
	switch(fmt) {
	case PIX_RGBA32:
		return 4;

	case PIX_FLOAT:
		return 4 * sizeof(float);

	default:
		break;
	}
	return 0;
}
