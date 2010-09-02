#ifndef HENGE_OPENGL_H_
#define HENGE_OPENGL_H_

#if defined(WIN32) || defined(__WIN32__)
#include <windows.h>
#endif

#include "int_types.h"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

#if defined(__MACH__) && defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#if defined(unix) || defined(__unix__) && !defined(__APPLE__)
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
#endif

#include "vmath.h"
#include "errlog.h"

#ifdef HENGE_OPENGL_CC_
#define extern
#endif

#ifdef __GNUC__
#define HENGE_FUNC_NAME		__PRETTY_FUNCTION__
#else
#define HENGE_FUNC_NAME		__FUNCTION__
#endif


#define glassert()					\
	do {							\
		GLenum err = glGetError();	\
		if(err != GL_NO_ERROR) {	\
			henge::error("OpenGL error: %s\n", henge::glstrerror(err));	\
			henge::error("%s:%d: %s\n", __FILE__, __LINE__, HENGE_FUNC_NAME);	\
			abort();				\
		}							\
	} while(0)

namespace henge {

extern struct GLCaps {
	bool multitex;
	bool glsl;
	bool vbo;
	bool fbo;
	bool mrt;
	bool trans_mat;
	bool psprites;
	bool sep_spec;
	bool gen_mipmaps;
	bool non_pow2_tex;
	bool aniso;
	int max_lights;
	int max_tex_units;
	int max_vattr;
	int max_aniso;
	int max_rt;
} caps;

bool init_opengl();

void store_matrix(Matrix4x4 *mat);
void load_matrix(const Matrix4x4 &mat);
void mult_matrix(const Matrix4x4 &mat);

const char *glstrerror(GLenum err);

}

#ifdef HENGE_OPENGL_CC_
#undef extern
#endif

#endif	// HENGE_OPENGL_H_
