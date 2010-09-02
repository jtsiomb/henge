#ifndef HENGE_SDR_H_
#define HENGE_SDR_H_

#include <vector>
#include "opengl.h"

// GLSL attribute slot used by the tangent vector
#define SDR_ATTR_TANGENT	12	// collision with MultiTexCoord4 shouldn't matter

namespace henge {

class Shader;

bool init_sdr();
void destroy_sdr();

bool add_shader(const char *vname, const char *pname, Shader *sdr);
Shader *get_shader(const char *vname, const char *pname);

// NULL pointer means: disable shaders
void set_shader(const Shader *sdr);

enum SdrType {
	SDR_VERTEX	= GL_VERTEX_SHADER_ARB,
	SDR_PIXEL	= GL_FRAGMENT_SHADER_ARB
};

class Shader {
private:
	unsigned int prog;
	unsigned int vsdr, psdr;

	mutable bool prog_valid;

	std::vector<std::string> uniforms;

	void find_uniforms();

public:
	Shader();
	~Shader();

	bool load_shader(const char *fname, SdrType type);
	bool compile_shader(const char *src, SdrType type, const char *fname = 0);
	bool link();

	bool set_uniform(const char *name, int val) const;
	bool set_uniform(const char *name, float val) const;
	bool set_uniform(const char *name, double val) const;
	bool set_uniform(const char *name, const Vector3 &vec) const;
	bool set_uniform(const char *name, const Vector4 &vec) const;
	bool set_uniform(const char *name, const Matrix4x4 &mat) const;

	bool bind() const;
};

}	// namespace henge

#endif	// HENGE_SDR_H_
