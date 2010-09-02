#ifndef HENGE_PIXMAP_H_
#define HENGE_PIXMAP_H_

namespace henge {

enum PixmapFormat {
	PIX_RGBA32,
	PIX_FLOAT
};

class Pixmap {
public:
	void *pixels;
	int xsz, ysz;

	PixmapFormat fmt;
	int bytes_per_pixel;	// derived from fmt

	bool manage_pixels;


	Pixmap();
	Pixmap(int xsz, int ysz, PixmapFormat fmt = PIX_RGBA32, void *pixels = 0);
	~Pixmap();

	bool set_pixels(int xsz, int ysz, PixmapFormat fmt = PIX_RGBA32, void *pixels = 0);

	bool load(const char *name);
	bool save(const char *name);

	bool generate(const char *expr);

	bool is_transparent() const;
};

void convert_pixmap(Pixmap *pix, PixmapFormat tofmt);

void copy_pixels(Pixmap *dest, Pixmap *src);
void copy_pixels(Pixmap *dest, int destx, int desty, Pixmap *src, int sx, int sy, int width, int height);

// counter-clockwise rotation
void rotate_pixmap(Pixmap *pix, int deg);

void mirror_pixmap(Pixmap *pix);
void flip_pixmap(Pixmap *pix);

}	// namespace henge

#endif	// HENGE_PIXMAP_H_
