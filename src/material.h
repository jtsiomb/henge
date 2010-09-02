#ifndef HENGE_MATERIAL_H_
#define HENGE_MATERIAL_H_

#include <string>
#include "color.h"
#include "texture.h"
#include "sdr.h"
#include "anim.h"

namespace henge {

enum MatAttr {
	MATTR_AMB_AND_DIF,
	MATTR_AMBIENT,		// ambient color \ generally tied together
	MATTR_DIFFUSE,		// diffuse color /
	MATTR_SPECULAR,		// specular color (blinn/phong)
	MATTR_EMISSION,		// emissive color (useless)
	MATTR_SHININESS,	// specular power (blinn/phong)
	MATTR_ROUGHNESS,	// surface roughness (tor/cook, oren/nayar)
	MATTR_IOR,			// index of refraction
	MATTR_ALPHA,		// alpha value

	MATTR_COUNT
};

enum {
	MAT_BIND_MATERIAL		= 1,
	MAT_BIND_TEXTURE		= 2,
	MAT_BIND_SHADER			= 4,
	MAT_BIND_TEXTURE_XFORM	= 8,
	MAT_BIND_ALL			= 15
};

void set_mat_bind_mask(unsigned int bmask);
unsigned int get_mat_bind_mask();

void set_global_alpha(float alpha);
float get_global_alpha();

#define MAT_MAX_TEX		4

class Material {
private:
	std::string name;
	Color attr[MATTR_COUNT];
	Shader *sdr;
	Texture *tex[MAT_MAX_TEX];

	XFormNode tex_xform;

public:
	Material(const Color &col = Color(0.5f, 0.5f, 0.5f, 1.0f));

	void set_name(const char *name);
	const char *get_name() const;

	void set_color(const Color &c, MatAttr ma = MATTR_AMB_AND_DIF);
	const Color &get_color(MatAttr ma = MATTR_AMB_AND_DIF) const;

	void set(float val, MatAttr ma);
	float get(MatAttr ma) const;

	void set_shader(Shader *sdr);
	Shader *get_shader();
	const Shader *get_shader() const;

	void set_texture(Texture *tex, int idx = 0);
	Texture *get_texture(int idx = 0);
	const Texture *get_texture(int idx = 0) const;

	const XFormNode *texture_xform() const;
	XFormNode *texture_xform();

	bool is_transparent() const;

	// setup opengl material & texture parameters
	void bind(unsigned int bind_mask = MAT_BIND_ALL) const;
};

}

#endif	// HENGE_MATERIAL_H_
