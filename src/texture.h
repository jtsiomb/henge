#ifndef HENGE_TEXTURE_H_
#define HENGE_TEXTURE_H_

#include "opengl.h"
#include "pixmap.h"

namespace henge {

class texture;

enum {
	TEX_GUESS,
	TEX_2D = GL_TEXTURE_2D,
	TEX_CUBE = GL_TEXTURE_CUBE_MAP_ARB
};

enum {TEX_ANISO_MAX = 0};

bool init_textures();
void destroy_textures();

// texture manager interface
texture *get_texture(const char *name, unsigned int textype = TEX_GUESS, unsigned int pixfmt = GL_RGBA);
const char *get_texture_name(const texture *tex);
bool add_texture(const char *name, texture *tex);
bool dump_textures(bool managed_only = false);

// render to texture interface
bool push_render_target();
bool pop_render_target();
bool set_render_target(texture *tex);
bool set_render_targets(int num, ...);

void set_texture(const texture *tex, int unit = 0);

// don't use this directly, included here just because MSVC chokes without the prototype
bool set_render_target_fbo(texture *tex, int mrt_idx);

// returns the next power of 2 (if x is a power of 2 it just returns x)
int round_pow2(int x);

class texture {
protected:
	unsigned int tid, rt_depth;
	mutable uint32_t *pixels;

	int xsz, ysz;		// width & height
	int uxsz, uysz;		// useful width & height (as opposed to padding)

	unsigned int textype, pixfmt;
	unsigned int wrap, min_filter, mag_filter;
	float aniso;

	bool transp;
	bool gen_mipmaps;

public:
	texture();
	virtual ~texture();

	virtual bool load(const char *fname) = 0;
	virtual bool save(const char *fname) = 0;
	virtual bool generate(const char *expr) = 0;

	void set_tex_id(unsigned int id);
	unsigned int get_tex_id() const;

	virtual void set_mipmap_gen(bool genmip);
	virtual bool get_mipmap_gen() const;

	virtual bool is_transparent() const;

	void set_pixel_format(unsigned int fmt);
	unsigned int get_pixel_format() const;

	void set_wrap(unsigned int wrap);
	unsigned int get_wrap() const;

	void set_filter(unsigned int min, unsigned int mag);
	unsigned int get_min_filter() const;
	unsigned int get_mag_filter() const;

	void set_max_anisotropy(float val);
	float get_max_anisotropy() const;

	void set_width(int w);
	void set_height(int h);
	int get_width() const;
	int get_height() const;

	void set_useful_area(int w, int h);
	int get_useful_width() const;
	int get_useful_height() const;

	virtual void copy_frame() = 0;
	virtual void copy_frame(int x, int y, int xsz, int ysz) = 0;

	void bind(int tunit = 0) const;

	friend bool henge::set_render_target_fbo(texture *tex, int mrt_idx);
};

class texture_2d : public texture {
public:
	texture_2d();
	virtual ~texture_2d();

	virtual void set_mipmap_gen(bool genmip);

	virtual bool load(const char *fname);
	virtual bool save(const char *fname);
	virtual bool generate(const char *expr);

	void set_image(const pixmap &img);
	void set_image(uint32_t *pixels, int x, int y);
	uint32_t *get_image() const;

	virtual void copy_frame();
	virtual void copy_frame(int x, int y, int xsz, int ysz);
};

class texture_cube : public texture {
protected:
	int size;
	bool face_tex_inited[6];

	unsigned int transp_cube;

public:
	texture_cube();
	virtual ~texture_cube();
	
	virtual bool is_transparent() const;

	virtual bool load(const char *fname);
	virtual bool load_cross(const char *fname);
	virtual bool save(const char *fname);
	virtual bool generate(const char *expr);

	void set_image(const pixmap &img, unsigned int face);
	void set_image(uint32_t *pixels, int size, unsigned int face);
	uint32_t *get_image(unsigned int face) const;

	int get_size() const;

	virtual void copy_frame();	// derived
	virtual void copy_frame(int face);	// new
	virtual void copy_frame(int x, int y, int xsz, int ysz);	// derived
	virtual void copy_frame(int x, int y, int xsz, int ysz, int face);	// new
};

// helper function, converts a heightfield to a normalmap
void heightmap_to_normalmap(texture_2d *tex, float scale_fact = 1.0f);

}

#endif	// HENGE_TEXTURE_H_
