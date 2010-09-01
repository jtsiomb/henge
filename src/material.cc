#include "material.h"
#include "unicache.h"

using namespace henge;

static unsigned int global_mask = 0xffffffff;
static float global_alpha = 1.0f;

void henge::set_mat_bind_mask(unsigned int bmask)
{
	global_mask = bmask;
}

unsigned int henge::get_mat_bind_mask()
{
	return global_mask;
}

void henge::set_global_alpha(float alpha)
{
	global_alpha = alpha;
}

float henge::get_global_alpha()
{
	return global_alpha;
}


material::material(const color &col)
{
	set_color(col, MATTR_AMB_AND_DIF);
	set_color(color(0, 0, 0, 1), MATTR_SPECULAR);
	set_color(color(0, 0, 0, 1), MATTR_EMISSION);
	set(0.0f, MATTR_SHININESS);
	set(0.0f, MATTR_ROUGHNESS);
	set(1.0f, MATTR_IOR);
	set(1.0f, MATTR_ALPHA);

	memset(tex, 0, MAT_MAX_TEX * sizeof *tex);
	sdr = 0;
}

void material::set_name(const char *name)
{
	this->name = name;
}

const char *material::get_name() const
{
	return name.c_str();
}

void material::set_color(const color &c, mat_attr ma)
{
	if(ma == MATTR_AMB_AND_DIF) {
		attr[MATTR_AMBIENT] = attr[MATTR_DIFFUSE] = c;
	} else {
		attr[ma] = c;
	}
}

const color &material::get_color(mat_attr ma) const
{
	return attr[ma == MATTR_AMB_AND_DIF ? MATTR_DIFFUSE : ma];
}

void material::set(float val, mat_attr ma)
{
	if(ma == MATTR_SHININESS && val > 128.0f) {
		val = 128.0f;
	}
	color c = color(val, val, val, 1.0f);

	if(ma == MATTR_AMB_AND_DIF) {
		attr[MATTR_AMBIENT] = attr[MATTR_DIFFUSE] = c;
	} else {
		attr[ma] = c;
	}
}

float material::get(mat_attr ma) const
{
	return attr[ma == MATTR_AMB_AND_DIF ? MATTR_DIFFUSE : ma].x;
}

void material::set_shader(shader *sdr)
{
	this->sdr = sdr;
}

shader *material::get_shader()
{
	return sdr;
}

const shader *material::get_shader() const
{
	return sdr;
}

void material::set_texture(texture *tex, int idx)
{
	this->tex[idx] = tex;
}

texture *material::get_texture(int idx)
{
	return tex[idx];
}

const texture *material::get_texture(int idx) const
{
	return tex[idx];
}

const xform_node *material::texture_xform() const
{
	return &tex_xform;
}

xform_node *material::texture_xform()
{
	return &tex_xform;
}

bool material::is_transparent() const
{
	if(attr[MATTR_ALPHA].x * global_alpha < 1.0 - SMALL_NUMBER) {
		return true;
	}

	if(tex[MATTR_DIFFUSE] && tex[MATTR_DIFFUSE]->is_transparent()) {
		return true;
	}

	return false;
}

// defined in vmath_config.h
#ifdef SINGLE_PRECISION_MATH
#define gl_matv(attr, vec)	glMaterialfv(GL_FRONT_AND_BACK, attr, (scalar_t*)&(vec))
#else
#define gl_matv(attr, vec)	glMaterialdv(GL_FRONT_AND_BACK, attr, (scalar_t*)&(vec))
#endif
#define gl_mat(attr, vec)	glMaterialf(GL_FRONT_AND_BACK, attr, (vec).x)

void material::bind(unsigned int bind_mask) const
{
	bind_mask &= global_mask;

	if(bind_mask & MAT_BIND_MATERIAL) {
		color dif = attr[MATTR_DIFFUSE];
		dif.w *= attr[MATTR_ALPHA].x * global_alpha;

		//gl_matv(GL_AMBIENT, attr[MATTR_AMBIENT]);
		gl_matv(GL_AMBIENT, dif);	// XXX remove
		gl_matv(GL_DIFFUSE, dif);
		gl_matv(GL_SPECULAR, attr[MATTR_SPECULAR]);
		gl_matv(GL_EMISSION, attr[MATTR_EMISSION]);
		gl_mat(GL_SHININESS, attr[MATTR_SHININESS]);

		cache_uniform("uc_mat_roughness", attr[MATTR_ROUGHNESS].x);
		cache_uniform("uc_mat_ior", attr[MATTR_IOR].x);
	}

	if(bind_mask & MAT_BIND_TEXTURE) {
		int tunit = 0;
		for(int i=0; i<MAT_MAX_TEX; i++) {
			if(tex[i]) {
				tex[i]->bind(tunit++);
			}
		}
	}

	if(bind_mask & MAT_BIND_TEXTURE_XFORM) {
		glMatrixMode(GL_TEXTURE);
		mult_matrix(tex_xform.get_xform_matrix());
	}

	if(bind_mask & MAT_BIND_SHADER) {
		if(sdr) {
			sdr->bind();
		}
	}


	if(is_transparent()) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.05f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}
