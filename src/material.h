#ifndef HENGE_MATERIAL_H_
#define HENGE_MATERIAL_H_

#include <string>
#include "color.h"
#include "texture.h"
#include "sdr.h"
#include "anim.h"

namespace henge {

enum mat_attr {
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

class material {
private:
	std::string name;
	color attr[MATTR_COUNT];
	shader *sdr;
	texture *tex[MAT_MAX_TEX];

	xform_node tex_xform;

public:
	material(const color &col = color(0.5f, 0.5f, 0.5f, 1.0f));

	void set_name(const char *name);
	const char *get_name() const;

	void set_color(const color &c, mat_attr ma = MATTR_AMB_AND_DIF);
	const color &get_color(mat_attr ma = MATTR_AMB_AND_DIF) const;

	void set(float val, mat_attr ma);
	float get(mat_attr ma) const;

	void set_shader(shader *sdr);
	shader *get_shader();
	const shader *get_shader() const;

	void set_texture(texture *tex, int idx = 0);
	texture *get_texture(int idx = 0);
	const texture *get_texture(int idx = 0) const;

	const xform_node *texture_xform() const;
	xform_node *texture_xform();

	bool is_transparent() const;

	// setup opengl material & texture parameters
	void bind(unsigned int bind_mask = MAT_BIND_ALL) const;
};

}

#endif	// HENGE_MATERIAL_H_
