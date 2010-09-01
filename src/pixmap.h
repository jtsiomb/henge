#ifndef HENGE_PIXMAP_H_
#define HENGE_PIXMAP_H_

namespace henge {

enum pixmap_format {
	PIX_RGBA32,
	PIX_FLOAT
};

class pixmap {
public:
	void *pixels;
	int xsz, ysz;

	pixmap_format fmt;
	int bytes_per_pixel;	// derived from fmt

	bool manage_pixels;


	pixmap();
	pixmap(int xsz, int ysz, pixmap_format fmt = PIX_RGBA32, void *pixels = 0);
	~pixmap();

	bool set_pixels(int xsz, int ysz, pixmap_format fmt = PIX_RGBA32, void *pixels = 0);

	bool load(const char *name);
	bool save(const char *name);

	bool generate(const char *expr);

	bool is_transparent() const;
};

void convert_pixmap(pixmap *pix, pixmap_format tofmt);

void copy_pixels(pixmap *dest, pixmap *src);
void copy_pixels(pixmap *dest, int destx, int desty, pixmap *src, int sx, int sy, int width, int height);

// counter-clockwise rotation
void rotate_pixmap(pixmap *pix, int deg);

void mirror_pixmap(pixmap *pix);
void flip_pixmap(pixmap *pix);

}	// namespace henge

#endif	// HENGE_PIXMAP_H_
