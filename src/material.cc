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


Material::Material(const Color &col)
{
	set_color(col, MATTR_AMB_AND_DIF);
	set_color(Color(0, 0, 0, 1), MATTR_SPECULAR);
	set_color(Color(0, 0, 0, 1), MATTR_EMISSION);
	set(0.0f, MATTR_SHININESS);
	set(0.0f, MATTR_ROUGHNESS);
	set(1.0f, MATTR_IOR);
	set(1.0f, MATTR_ALPHA);

	memset(tex, 0, MAT_MAX_TEX * sizeof *tex);
	sdr = 0;
}

void Material::set_name(const char *name)
{
	this->name = name;
}

const char *Material::get_name() const
{
	return name.c_str();
}

void Material::set_color(const Color &c, MatAttr ma)
{
	if(ma == MATTR_AMB_AND_DIF) {
		attr[MATTR_AMBIENT] = attr[MATTR_DIFFUSE] = c;
	} else {
		attr[ma] = c;
	}
}

const Color &Material::get_color(MatAttr ma) const
{
	return attr[ma == MATTR_AMB_AND_DIF ? MATTR_DIFFUSE : ma];
}

void Material::set(float val, MatAttr ma)
{
	if(ma == MATTR_SHININESS && val > 128.0f) {
		val = 128.0f;
	}
	Color c = Color(val, val, val, 1.0f);

	if(ma == MATTR_AMB_AND_DIF) {
		attr[MATTR_AMBIENT] = attr[MATTR_DIFFUSE] = c;
	} else {
		attr[ma] = c;
	}
}

float Material::get(MatAttr ma) const
{
	return attr[ma == MATTR_AMB_AND_DIF ? MATTR_DIFFUSE : ma].x;
}

void Material::set_shader(Shader *sdr)
{
	this->sdr = sdr;
}

Shader *Material::get_shader()
{
	return sdr;
}

const Shader *Material::get_shader() const
{
	return sdr;
}

void Material::set_texture(Texture *tex, int idx)
{
	this->tex[idx] = tex;
}

Texture *Material::get_texture(int idx)
{
	return tex[idx];
}

const Texture *Material::get_texture(int idx) const
{
	return tex[idx];
}

const XFormNode *Material::texture_xform() const
{
	return &tex_xform;
}

XFormNode *Material::texture_xform()
{
	return &tex_xform;
}

bool Material::is_transparent() const
{
	if(attr[MATTR_ALPHA].x * global_alpha < 1.0 - SMALL_NUMBER) {
		return true;
	}

	if(tex[MATTR_DIFFUSE] && tex[MATTR_DIFFUSE]->is_transparent()) {
		return true;
	}

	return false;
}

#define gl_matv(attr, vec)	\
	do { \
		float fvec[] = {(vec).x, (vec).y, (vec).z, 1.0}; \
		glMaterialfv(GL_FRONT_AND_BACK, attr, fvec); \
	} while(0)
#define gl_mat(attr, vec)	glMaterialf(GL_FRONT_AND_BACK, attr, (vec).x)

void Material::bind(unsigned int bind_mask) const
{
	bind_mask &= global_mask;

	if(bind_mask & MAT_BIND_MATERIAL) {
		Color dif = attr[MATTR_DIFFUSE];
		dif.w *= attr[MATTR_ALPHA].x * global_alpha;

		gl_matv(GL_AMBIENT, dif);	// XXX remove
		gl_matv(GL_DIFFUSE, dif);
		gl_matv(GL_SPECULAR, attr[MATTR_SPECULAR]);
		gl_matv(GL_EMISSION, attr[MATTR_EMISSION]);
		gl_mat(GL_SHININESS, attr[MATTR_SHININESS]);

		cache_uniform("uc_mat_roughness", (float)attr[MATTR_ROUGHNESS].x);
		cache_uniform("uc_mat_ior", (float)attr[MATTR_IOR].x);
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
