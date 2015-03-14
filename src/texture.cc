#include <map>
#include <string>
#include <stack>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include "opengl.h"
#include "texture.h"
#include "pixmap.h"
#include "color.h"
#include "errlog.h"
#include "henge.h"

#include <sys/stat.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

// tests if an OpenGL pixel format is floating point
#define IS_FLOAT_FMT(fmt)	\
	((fmt) == GL_RGBA32F_ARB || (fmt) == GL_RGBA16F_ARB || \
	 (fmt) == GL_RGB32F_ARB || (fmt) == GL_RGB16F_ARB)

using namespace std;
using namespace henge;

namespace henge {
	bool set_render_target_fbo(Texture *tex, int mrt_idx);
	bool set_render_target_copy(Texture *tex, int mrt_idx);
}

static map<string, Texture*> texman;

// a set of render targets is the element of the rt-stack
struct RTSet {
	int num;
	Texture *rt[8];

	RTSet() { num = 0; }
};

static bool set_render_target_set(const RTSet &set);

static stack<RTSet> rt;
static unsigned int fbo;


bool henge::init_textures()
{
	if(getenv("HENGE_NO_FBO")) {
		caps.fbo = 0;
	}

	if(caps.fbo) {
		glGenFramebuffersEXT(1, &fbo);
	}

	rt.push(RTSet());	// initial render target set is empty (regular framebuffer)
	get_viewport(0, 0, 0, 0);	// make sure the internal viewport is updated

	return true;
}

void henge::destroy_textures()
{
	if(caps.fbo) {
		glDeleteFramebuffersEXT(1, &fbo);
	}

	while(rt.size()) {
		rt.pop();
	}

	map<string, Texture*>::iterator iter = texman.begin();
	while(iter != texman.end()) {
		delete iter++->second;
	}
	texman.clear();
}

/* texture manager - get_texture
 *
 * Requests a texture, identified by name, from the texture manager.
 *
 * If a texture is not found associated with that name in the database,
 * the name will be treated as a filename and an attempt will be made
 * to load it from the filesystem and add it to the database. Otherwise
 * a pointer to the existing texture will be returned.
 *
 * TODO: At this point textures stay in the texture manager database for ever.
 *	   implement texture sets if the need arises.
 * TODO: Should be fun to encode texture generators in the "name" string and have
 *	   textures be generated on demand and requested through the same interface.
 */
Texture *henge::get_texture(const char *name, unsigned int textype, unsigned int pixfmt)
{
	const char *prefix = IS_FLOAT_FMT(pixfmt) ? "float_" : "rgba32_";
	char *dbname = (char*)alloca(strlen(name) + strlen(prefix) + 1);
	sprintf(dbname, "%s%s", prefix, name);

	map<string, Texture*>::iterator iter = texman.find(dbname);
	if(iter != texman.end()) {
		return iter->second;
	}

	Texture *tex;

	switch(textype) {
	case TEX_2D:
		tex = new Texture2D;
		break;

	case TEX_CUBE:
		tex = new TextureCube;
		break;

	default:
		// try to guess...
		if(strstr(name, ".cube")) {
			tex = new TextureCube;
		} else {
			tex = new Texture2D;
		}
	}

	tex->set_pixel_format(pixfmt);

	if(strstr(name, "texgen:") == name) {
		info("generating texture: %s\n", name + strlen("texgen:"));
		if(!tex->generate(name + strlen("texgen:"))) {
			delete tex;
			return 0;
		}
	} else {
		info("loading texture: %s\n", name);
		if(!tex->load(name)) {
			delete tex;
			return 0;
		}
	}

	add_texture(dbname, tex);
	return tex;
}

const char *henge::get_texture_name(const Texture *tex)
{
	map<string, Texture*>::iterator iter = texman.begin();
	while(iter != texman.end()) {
		if(iter->second == tex) {
			return iter->first.c_str();
		}
		iter++;
	}
	return 0;
}

bool henge::add_texture(const char *name, Texture *tex)
{
	try {
		texman[name] = tex;
	}
	catch(...) {
		return false;
	}
	return true;
}


bool henge::dump_textures(bool managed_only)
{
#if !defined(WIN32) && !defined(__WIN32__)
#define TEX_DUMP_DIR	"texdump"

	// create the texture dump directory
	if(mkdir(TEX_DUMP_DIR, 0755) == -1) {
		if(errno == EEXIST) {
			struct stat st;
			if(stat(TEX_DUMP_DIR, &st) == -1 || !S_ISDIR(st.st_mode)) {
				return false;
			}
		} else {
			return false;
		}
	}
#else
#define TEX_DUMP_DIR	"."
#endif

	//if(managed_only) {
		map<string, Texture*>::iterator iter = texman.begin();
		while(iter != texman.end()) {
			const char *name = iter->first.c_str();
			Texture *tex = (iter++)->second;

			char *pname = new char[strlen(TEX_DUMP_DIR) + strlen(name) + 2];
			sprintf(pname, "%s/%s", TEX_DUMP_DIR, name);

			if(tex) tex->save(pname);
			delete [] pname;
		}
		return true;
	/*}

	char *pname = (char*)alloca(strlen(TEX_DUMP_DIR) + 6);
	Texture2D tex;

	for(int i=0; i<2048; i++) {
		if(!glIsTexture(i))
			continue;

		glBindTexture(GL_TEXTURE_2D, i);

		tex.set_tex_id(i);

		int xsz, ysz;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &xsz);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &ysz);
		tex.set_width(xsz);
		tex.set_height(ysz);

		sprintf(pname, "%s/%04d.png", TEX_DUMP_DIR, i);
		tex.save(pname);
	}
	return true;
	*/
}


bool henge::push_render_target()
{
	try {
		if(rt.empty()) {
			rt.push(RTSet());
		} else {
			rt.push(rt.top());
		}
	}
	catch(...) {
		return false;
	}
	return true;
}

bool henge::pop_render_target()
{
	if(rt.empty()) {
		return false;
	}

	rt.pop();
	return set_render_target_set(rt.top());
}

static bool (*setrt_func[])(Texture*, int) = {
	set_render_target_copy,
	set_render_target_fbo
};

bool henge::set_render_target(Texture *tex)
{
	RTSet set;
	set.num = 1;
	set.rt[0] = tex;

	return set_render_target_set(set);
}

bool henge::set_render_targets(int num, ...)
{
	static GLenum draw_bufs[] = {
		GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
		GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT,
		GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT,
		GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT
	};

	va_list ap;
	RTSet set;
	set.num = num;

	va_start(ap, num);
	for(int i=0; i<num; i++) {
		set.rt[i] = va_arg(ap, Texture*);
	}
	va_end(ap);

	if(!set_render_target_set(set)) {
		return false;
	}
	if(num > 1) {
		glDrawBuffersARB(num, draw_bufs);
	}
	return true;
}

static bool set_render_target_set(const RTSet &set)
{
	rt.top() = set;

	if(!set.num) {
		setrt_func[(int)caps.fbo](0, 0);
		// XXX should we call glDrawBuffer(GL_BACK_LEFT) or something?
		return true;
	}

	for(int i=0; i<set.num; i++) {
		setrt_func[(int)caps.fbo](set.rt[i], i);
	}
	return true;
}

#ifndef NDEBUG
static const char *fbstat_str[] = {
	"GL_FRAMEBUFFER_COMPLETE",
	"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT",
	"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT",
	"no such fbo error",
	"GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS",
	"GL_FRAMEBUFFER_INCOMPLETE_FORMATS",
	"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER",
	"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER",
	"GL_FRAMEBUFFER_UNSUPPORTED"
};
#endif

#define ASSERT_GLERROR	\
	do {	\
		GLenum err;	\
		if((err = glGetError()) != GL_NO_ERROR) {	\
			error("%s: %d - GL ERROR: %s (%d)\n", __FILE__, __LINE__, gluErrorString(err), err);	\
			abort();	\
		}	\
	} while(0)

bool henge::set_render_target_fbo(Texture *tex, int mrt_idx)
{
	if(!caps.mrt && mrt_idx > 0) {
		static bool emmited_error;
		if(!emmited_error) {
			error("set_render_target called with mrt_idx = %d, but MRT is not available!\n", mrt_idx);
			emmited_error = true;
		}
		return false;
	}

	if(!tex) {
		// tex is null, reverting to standard framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		int x, y, xsz, ysz;
		get_viewport(&x, &y, &xsz, &ysz);
		glViewport(x, y, xsz, ysz);
	} else {
		// if this texture doesn't have a depth buffer rendertarget, create it
		if(!tex->rt_depth) {
			glGenRenderbuffersEXT(1, &tex->rt_depth);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, tex->rt_depth);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, tex->xsz, tex->ysz);
		}

		// bind the global fbo and attach the color and depth buffers
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + mrt_idx,
				GL_TEXTURE_2D, tex->tid, 0);

		if(mrt_idx == 0) {
			// only attach the depth buffer once for mrt rendertargets
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
					GL_RENDERBUFFER_EXT, tex->rt_depth);
			// TODO add stencil attachment
		}

#ifndef NDEBUG
		int fbstat = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(fbstat != GL_FRAMEBUFFER_COMPLETE_EXT) {
			warning("incomplete framebuffer object: %u (%s)\n", fbo,
					fbstat_str[fbstat - GL_FRAMEBUFFER_COMPLETE_EXT]);
		}
#endif
		glViewport(0, 0, tex->get_useful_width(), tex->get_useful_height());
	}

	assert(glGetError() == 0);
	return true;
}

/* Set_render_target_copy will try to fake set_render_target by copying the
 * framebuffer to a texture after rendering takes place (when the regular
 * framebuffer is reset). The semantics aren't identical, as the regular render
 * target will be dirty after this. Also although I could make it work with
 * multiple levels of push and pop, I won't bother.
 *
 * Ignores the mrt_idx parameter, if you're trying to do mrt without proper
 * render targets, you're clearly mad.
 */
bool henge::set_render_target_copy(Texture *tex, int mrt_idx)
{
	static Texture *prev;

	if(prev && prev != tex) {
		prev->copy_frame(0, 0, prev->get_useful_width(), prev->get_useful_height());
	}
	prev = tex;
	return true;
}


void henge::set_texture(const Texture *tex, int unit)
{
	if(tex) {
		tex->bind(unit);
	} else {
		if(caps.multitex) {
			glActiveTextureARB(GL_TEXTURE0_ARB + unit);
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	}
}

int henge::round_pow2(int x)
{
	x--;
	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;
	return x + 1;
}

// -- texture abstract base class

Texture::Texture()
{
	pixels = 0;
	glGenTextures(1, &tid);
	transp = false;
	set_mipmap_gen(true);
	pixfmt = 4;
	wrap = GL_REPEAT;
	min_filter = GL_LINEAR_MIPMAP_LINEAR;
	mag_filter = GL_LINEAR;
	aniso = 1.0;

	xsz = ysz = 0;
	uxsz = uysz = -1;

	/* a depth buffer fbo attachment will be allocated on demand
	 * when this texture is first bound as a render target
	 */
	rt_depth = 0;
}

Texture::~Texture()
{
	if(pixels) {
		delete [] pixels;
	}
	glDeleteTextures(1, &tid);

	if(rt_depth) {
		glDeleteRenderbuffersEXT(1, &rt_depth);
	}
}

void Texture::set_tex_id(unsigned int id)
{
	tid = id;
}

unsigned int Texture::get_tex_id() const
{
	return tid;
}

void Texture::set_mipmap_gen(bool genmip)
{
	gen_mipmaps = genmip;
}

bool Texture::get_mipmap_gen() const
{
	return gen_mipmaps;
}

bool Texture::is_transparent() const
{
	return transp;
}

void Texture::set_pixel_format(unsigned int fmt)
{
	pixfmt = fmt;
}

unsigned int Texture::get_pixel_format() const
{
	return pixfmt;
}

void Texture::set_wrap(unsigned int wrap)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);
	glTexParameteri(textype, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(textype, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(textype, GL_TEXTURE_WRAP_R, wrap);
	glPopAttrib();

	this->wrap = wrap;
}

unsigned int Texture::get_wrap() const
{
	return wrap;
}

void Texture::set_filter(unsigned int min, unsigned int mag)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);
	glTexParameteri(textype, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, mag);
	glPopAttrib();

	min_filter = min;
	mag_filter = mag;
}

unsigned int Texture::get_min_filter() const
{
	return min_filter;
}


unsigned int Texture::get_mag_filter() const
{
	return mag_filter;
}

void Texture::set_max_anisotropy(float val)
{
	if(caps.aniso) {
		if(val < 1.0) val = (float)caps.max_aniso;

		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(textype, tid);
		glTexParameterf(textype, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
		glPopAttrib();

		aniso = val;
	}
}

float Texture::get_max_anisotropy() const
{
	return aniso;
}

void Texture::set_width(int w)
{
	xsz = w;
}

void Texture::set_height(int h)
{
	ysz = h;
}

int Texture::get_width() const
{
	return xsz;
}

int Texture::get_height() const
{
	return ysz;
}

void Texture::set_useful_area(int w, int h)
{
	uxsz = w;
	uysz = h;
}

int Texture::get_useful_width() const
{
	return uxsz == -1 ? xsz : uxsz;
}

int Texture::get_useful_height() const
{
	return uysz == -1 ? ysz : uysz;
}

void Texture::bind(int tunit) const
{
	static bool dirty_texmat;

	/* if we have useless padding, setup the texture matrix to crop it out
	 * useful for presenting non-pow2 pictures and rtt framebuffer images
	 *
	 * NOTE: if we do this, we're also setting the dirty texmat flag, and we're
	 *       setting the 3rd diagonal element to (float)0xbad. If both these
	 *       conditions hold next time around we'll reset the matrix.
	 */

	if(uxsz != -1 && uysz != -1) {
		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef((float)uxsz / (float)xsz, (float)uysz / (float)ysz, (float)0xbad);
		glPopAttrib();
		//info("DBG tex::bind set uxsz/uysz texmat\n");

		dirty_texmat = true;
	} else {
		if(dirty_texmat) {
			float mat[16];
			glGetFloatv(GL_TEXTURE_MATRIX, mat);

			if(fabs(mat[10] - (float)0xbad) < SMALL_NUMBER) {
				glPushAttrib(GL_TRANSFORM_BIT);
				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glPopAttrib();
				//info("DBG tex::bind unset uxsz/uysz texmat\n");
			}
			dirty_texmat = false;
		}
	}

	if(caps.multitex) {
		glActiveTextureARB(GL_TEXTURE0_ARB + tunit);
	}
	glEnable(textype);
	glBindTexture(textype, tid);

	if(caps.multitex) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
}

// -- 2d texture class

Texture2D::Texture2D()
{
	xsz = ysz = -1;
	textype = GL_TEXTURE_2D;

	set_mipmap_gen(true);

	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);
	glTexParameteri(textype, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(textype, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(textype, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPopAttrib();
}

Texture2D::~Texture2D()
{
}

void Texture2D::set_mipmap_gen(bool genmip)
{
	Texture::set_mipmap_gen(genmip);

	if(caps.gen_mipmaps) {
		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(GL_TEXTURE_2D, tid);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, genmip ? 1 : 0);
		glPopAttrib();
	}
}

bool Texture2D::load(const char *fname)
{
	Pixmap img;

	if(IS_FLOAT_FMT(pixfmt)) {
		img.fmt = PIX_FLOAT;
	}

	if(!img.load(fname)) {
		return false;
	}
	set_image(img);
	return true;
}


static PixmapFormat gl2pixfmt(unsigned int fmt)
{
	if(IS_FLOAT_FMT(fmt)) {
		return PIX_FLOAT;
	}
	return PIX_RGBA32;
}

bool Texture2D::save(const char *fname)
{
	if(memcmp(fname, ".cube", 6) == 0) {
		return false;
	}

	Pixmap img;
	if(!img.set_pixels(xsz, ysz, gl2pixfmt(pixfmt), get_image())) {
		return false;
	}
	return img.save(fname);
}

bool Texture2D::generate(const char *expr)
{
	Pixmap img;
	if(!img.generate(expr)) {
		return false;
	}
	set_image(img);
	return true;
}

static unsigned int glpixfmt(PixmapFormat fmt)
{
	switch(fmt) {
	case PIX_RGBA32:
		return GLPIXFMT;	// defined in byteorder.h

	case PIX_FLOAT:
		return GL_RGBA;

	default:
		error("glpixfmt called with invalid format: %d\n", (int)fmt);
		break;
	}
	return 0;
}

static unsigned int glpixtype(PixmapFormat fmt)
{
	switch(fmt) {
	case PIX_RGBA32:
		return GL_UNSIGNED_BYTE;

	case PIX_FLOAT:
		return GL_FLOAT;

	default:
		error("glpixtype called with invalid format: %d\n", (int)fmt);
		break;
	}
	return 0;
}

void Texture2D::set_image(const Pixmap &img)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);

	if(img.is_transparent()) {
		transp = true;
	}

	if(img.xsz != xsz || img.ysz != ysz) {
		if(this->pixels) {
			delete [] this->pixels;
			this->pixels = 0;
		}
		xsz = img.xsz;
		ysz = img.ysz;

		if(img.pixels && gen_mipmaps && !caps.gen_mipmaps) {
			gluBuild2DMipmaps(GL_TEXTURE_2D, pixfmt, img.xsz, img.ysz, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
		} else {
			glTexImage2D(textype, 0, pixfmt, img.xsz, img.ysz, 0, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
		}
	} else {
		if(img.pixels) {
			if(gen_mipmaps && !caps.gen_mipmaps) {
				gluBuild2DMipmaps(GL_TEXTURE_2D, pixfmt, img.xsz, img.ysz, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
			} else {
				// faster version, no need to re-create the Texture2D, just update the pixels
				glTexSubImage2D(textype, 0, 0, 0, img.xsz, img.ysz, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
			}
		}
	}

	glPopAttrib();
}

void Texture2D::set_image(uint32_t *pixels, int x, int y)
{
	Pixmap img;
	img.pixels = pixels;
	img.xsz = x;
	img.ysz = y;
	img.fmt = PIX_RGBA32;

	set_image(img);
	img.pixels = 0;
}

uint32_t *Texture2D::get_image() const
{
	if(!pixels) {
		try {
			pixels = new uint32_t[xsz * ysz];
		}
		catch(...) {
			return 0;
		}
	} else {
		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(textype, tid);
		glGetTexImage(textype, 0, GLPIXFMT, GL_UNSIGNED_BYTE, pixels);
		glPopAttrib();
	}
	return pixels;
}


void Texture2D::copy_frame()
{
	copy_frame(0, 0, xsz, ysz);
}

void Texture2D::copy_frame(int x, int y, int xsz, int ysz)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(GL_TEXTURE_2D, tid);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, xsz, ysz);
	glPopAttrib();
}

// -- cubemap class

TextureCube::TextureCube()
{
	textype = GL_TEXTURE_CUBE_MAP_ARB;
	transp_cube = 0;

	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);
	glTexParameteri(textype, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(textype, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(textype, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(textype, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(textype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPopAttrib();

	memset(face_tex_inited, 0, sizeof face_tex_inited);
}

TextureCube::~TextureCube()
{
}

bool TextureCube::is_transparent() const
{
	return transp_cube != 0;
}

bool TextureCube::load(const char *fname)
{
	char buf[PATH_MAX];
	if(!find_file(fname, buf)) {
		return false;
	}

	FILE *fp;
	if(!(fp = fopen(buf, "rb"))) {
		return false;
	}

	// check magic
	if(!fgets(buf, sizeof buf, fp) || strstr(buf, "CUBE") != buf) {
		fclose(fp);
		return load_cross(fname);
	}

	for(int i=0; i<6; i++) {
		if(!fgets(buf, sizeof buf, fp)) {
			fclose(fp);
			return false;
		}

		char *p;
		if((p = strchr(buf, '\r')) || (p = strchr(buf, '\n'))) {
			*p = 0;
		}

		Pixmap img;

		if(IS_FLOAT_FMT(pixfmt)) {
			img.fmt = PIX_FLOAT;
		}

		if(!img.load(buf)) {
			fclose(fp);
			return false;
		}
		set_image(img, i + GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
	}

	fclose(fp);
	return true;
}

static int get_xoffs(int face, int xsz)
{
	switch(face) {
	case 0:	/* +X */
		return 2 * xsz / 3;
	case 1:	/* -X */
		return 0;
	default:
		break;
	}
	return xsz / 3;	/* everything else (vertical bit) */
}

static int get_yoffs(int face, int ysz)
{
	switch(face) {
	case 2:	/* +Y */
		return 0;
	case 3:	/* -Y */
		return ysz / 2;
	case 5:	/* -Z */
		return 3 * ysz / 4;
	default:
		break;
	}
	return ysz / 4;	/* +X, -X, +Z */
}

/*     +---+      load_cube_cross(filename)
 *     |+Y |      loads a cubemap from an image
 * +---+---+---+  file, assumed to contain the
 * |-X |+Z |+X |  cube face images in a vertical
 * +---+---+---+  cross layout.
 *     |-Y |
 *     +---+ \
 *     |-Z |  } last face rotated 180deg
 *     +---+ /
 */
bool TextureCube::load_cross(const char *fname)
{
	int size;

	Pixmap img;

	if(IS_FLOAT_FMT(pixfmt)) {
		img.fmt = PIX_FLOAT;
	}

	if(!img.load(fname)) {
		return false;
	}

	if(img.xsz / 3 != img.ysz / 4) {
		return false;
	}
	size = img.ysz / 4;

	for(int i=0; i<6; i++) {
		int xoffs = get_xoffs(i, img.xsz);
		int yoffs = get_yoffs(i, img.ysz);

		Pixmap fimg;
		fimg.set_pixels(size, size, img.fmt, 0);

		copy_pixels(&fimg, 0, 0, &img, xoffs, yoffs, size, size);
		if(i == 5) {
			rotate_pixmap(&fimg, 180);
		}

		set_image(fimg, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i);
		glassert();
	}

	return true;
}

bool TextureCube::save(const char *fname)
{
	char *face_fname = (char*)alloca(strlen(fname) + 4);

	Pixmap img;

	for(int i=0; i<6; i++) {
		uint32_t *pix = get_image(i + GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);

		char sign = i % 2 ? 'n' : 'p';
		char axis = i / 3 + 'x';
		sprintf(face_fname, "%c%c_%s", sign, axis, fname);

		if(!img.set_pixels(xsz, ysz, gl2pixfmt(pixfmt), pix)) {
			return false;
		}
		if(!img.save(face_fname)) {
			return false;
		}
	}
	return true;
}

bool TextureCube::generate(const char *expr)
{
	return false;	// TODO
}

void TextureCube::set_image(const Pixmap &img, unsigned int face)
{
	if(face < GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB || face > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB) {
		error("invalid face passed to TextureCube::set_image: %d\n", face);
		return;
	}

	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(textype, tid);

	int fidx = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;

	if(img.is_transparent()) {
		transp_cube |= 1 << fidx;
	}

	if(size != img.xsz) {
		if(this->pixels) {
			delete [] this->pixels;
			this->pixels = 0;
		}
		xsz = ysz = size = img.xsz;
		memset(face_tex_inited, 0, sizeof face_tex_inited);
	}

	if(!face_tex_inited[fidx]) {
		glTexImage2D(face, 0, pixfmt, size, size, 0, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
		glassert();
		face_tex_inited[fidx] = true;
	} else {
		// faster version, no need to re-create the Texture2D, just update the pixels
		glTexSubImage2D(face, 0, 0, 0, size, size, glpixfmt(img.fmt), glpixtype(img.fmt), img.pixels);
		glassert();
	}

	glPopAttrib();
}

void TextureCube::set_image(uint32_t *pixels, int size, unsigned int face)
{
	Pixmap img;
	img.pixels = pixels;
	img.xsz = img.ysz = size;
	img.fmt = PIX_RGBA32;

	set_image(img, face);
	img.pixels = 0;
}

uint32_t *TextureCube::get_image(unsigned int face) const
{
	if(!pixels) {
		try {
			pixels = new uint32_t[xsz * ysz];
		}
		catch(...) {
			return 0;
		}
	} else {
		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(textype, tid);
		glGetTexImage(face, 0, GLPIXFMT, GL_UNSIGNED_BYTE, pixels);
		glPopAttrib();
	}

	return pixels;
}

int TextureCube::get_size() const
{
	assert(xsz == ysz);
	return xsz;
}

void TextureCube::copy_frame()
{
	copy_frame(0, 0, xsz, ysz, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
}

void TextureCube::copy_frame(int face)
{
	copy_frame(0, 0, xsz, ysz, face);
}

void TextureCube::copy_frame(int x, int y, int xsz, int ysz)
{
	copy_frame(0, 0, xsz, ysz, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
}

void TextureCube::copy_frame(int x, int y, int xsz, int ysz, int face)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tid);
	glCopyTexSubImage2D(face, 0, 0, 0, x, y, xsz, ysz);
	glPopAttrib();
}

void henge::heightmap_to_normalmap(Texture2D *tex, float scale_fact)
{
	uint32_t *src = tex->get_image();
	int xsz = tex->get_width();
	int ysz = tex->get_height();

	uint32_t *norm_pixels = new uint32_t[xsz * ysz];
	uint32_t *dst = norm_pixels;

	for(int j=0; j<ysz; j++) {
		for(int i=0; i<xsz; i++) {
			int x0 = i > 0 ? -1 : 0;
			int y0 = j > 0 ? -xsz : 0;
			int x1 = i < xsz - 1 ? 1 : 0;
			int y1 = j < ysz - 1 ? xsz : 0;

			float dfdx = (float)(src[x0] & 0xff) / 255.0f - (float)(src[x1] & 0xff) / 255.0f;
			float dfdy = (float)(src[y0] & 0xff) / 255.0f - (float)(src[y1] & 0xff) / 255.0f;

			Vector3 n(dfdx * scale_fact, dfdy * scale_fact, 1);
			n.normalize();
			n = (n + 1.0f) * 127.5f;

			*dst++ = pack_color((int)n.x, (int)n.y, (int)n.z, 255);
			src++;
		}
	}

	tex->set_image(norm_pixels, xsz, ysz);
}
