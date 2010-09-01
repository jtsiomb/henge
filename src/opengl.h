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

extern struct gl_caps {
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

/*
// ARB multitexture
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

// ARB vertex buffer object
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;

// ARB transpose matrix
extern PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrixfARB;
extern PFNGLMULTTRANSPOSEMATRIXFARBPROC glMultTransposeMatrixfARB;

// GLSL
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC glUniform1iARB;
extern PFNGLUNIFORM1FARBPROC glUniform1fARB;
extern PFNGLUNIFORM3FARBPROC glUniform3fARB;
extern PFNGLUNIFORM4FARBPROC glUniform4fARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
extern PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB;
extern PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
extern PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB;

// EXT framebuffer object
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;

// ARB draw buffers
extern PFNGLDRAWBUFFERSARBPROC glDrawBuffersARB;
*/
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
