#define HENGE_OPENGL_CC_

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "opengl.h"
#include "errlog.h"
#include "unicache.h"
#include "texture.h"

using namespace henge;


static bool get_caps();
static bool load_extensions();
static bool have_ext(const char *ext_str, const char *name);


bool henge::init_opengl()
{
	if(!get_caps()) {
		return false;
	}
	if(!load_extensions()) {
		return false;
	}

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	if(caps.sep_spec) {
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	}

	return true;
}


void henge::store_matrix(Matrix4x4 *mat)
{
	int mmode;
	glGetIntegerv(GL_MATRIX_MODE, &mmode);

	int mget = GL_MODELVIEW_MATRIX + (mmode - GL_MODELVIEW);
#ifdef SINGLE_PRECISION_MATH
	glGetFloatv(mget, (float*)mat);
#else
	glGetDoublev(mget, (double*)mat);
#endif

	mat->transpose();
}

void henge::load_matrix(const Matrix4x4 &mat)
{
#ifdef SINGLE_PRECISION_MATH
	if(caps.trans_mat) {
		glLoadTransposeMatrixfARB((float*)&mat);
	} else {
		Matrix4x4 trans = mat.transposed();
		glLoadMatrixf((float*)&trans);
	}
#else
	if(caps.trans_mat) {
		glLoadTransposeMatrixdARB((double*)&mat);
	} else {
		Matrix4x4 trans = mat.transposed();
		glLoadMatrixd((double*)&trans);
	}
#endif
}


void henge::mult_matrix(const Matrix4x4 &mat)
{
#ifdef SINGLE_PRECISION_MATH
	if(caps.trans_mat) {
		glMultTransposeMatrixfARB((float*)&mat);
	} else {
		Matrix4x4 trans = mat.transposed();
		glMultMatrixf((float*)&trans);
	}
#else
	if(caps.trans_mat) {
		glMultTransposeMatrixdARB((double*)&mat);
	} else {
		Matrix4x4 trans = mat.transposed();
		glMultMatrixd((double*)&trans);
	}
#endif
}

static const char *error_str[] = {
	"GL_INVALID_ENUM",
	"GL_INVALID_VALUE",
	"GL_INVALID_OPERATION",
	"GL_STACK_OVERFLOW",
	"GL_STACK_UNDERFLOW",
	"GL_OUT_OF_MEMORY"
};

const char *henge::glstrerror(GLenum err)
{
	if(!err) {
		return "no error";
	}
	if(err < GL_INVALID_ENUM || err > GL_OUT_OF_MEMORY) {
		return "unknown error";
	}
	return error_str[err - GL_INVALID_ENUM];
}

// all the extensions we wish to check and set the appropriate caps flags
static struct {
	const char *ext;
	bool *cap;
	const char *desc;
} ext_caps[] = {
	{"GL_ARB_multitexture", &caps.multitex, "multitexturing"},
	{"GL_ARB_fragment_shader", &caps.glsl, "OpenGL shading language"},
	{"GL_ARB_vertex_buffer_object", &caps.vbo, "vertex buffer object"},
	{"GL_EXT_framebuffer_object", &caps.fbo, "framebuffer object"},
	{"GL_ARB_draw_buffers", &caps.mrt, "multiple render targets"},
	{"GL_ARB_transpose_matrix", &caps.trans_mat, "transpose matrix"},
	{"GL_ARB_point_parameters", &caps.psprites, "point sprites"},
	{"GL_EXT_separate_specular_color", &caps.sep_spec, "separate specular color"},
	{"GL_SGIS_generate_mipmap", &caps.gen_mipmaps, "mipmap generation"},
	{"GL_ARB_texture_non_power_of_two", &caps.non_pow2_tex, "non power of 2 textures"},
	{"GL_EXT_texture_filter_anisotropic", &caps.aniso, "anisotropic filtering"},
	{0, 0, 0}
};

static bool get_caps()
{
	const char *ext_str = (char*)glGetString(GL_EXTENSIONS);
	if(!ext_str) {
		error("failed to get the extensions string\n");
		return false;
	}

	for(int i=0; ext_caps[i].ext; i++) {
		if(have_ext(ext_str, ext_caps[i].ext)) {
			*ext_caps[i].cap = true;
		}

		info("%s: %s\n", ext_caps[i].desc, *ext_caps[i].cap ? "yes" : "no");
	}

	if(caps.multitex) {
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &caps.max_tex_units);
		info("texture units: %d\n", caps.max_tex_units);
	} else {
		caps.max_tex_units = 1;
	}

	glGetIntegerv(GL_MAX_LIGHTS, &caps.max_lights);
	info("maximum lights: %d\n", caps.max_lights);

	if(caps.glsl) {
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &caps.max_vattr);
		info("maximum vertex attribs: %d\n", caps.max_vattr);
	} else {
		caps.max_vattr = 0;
	}

	if(caps.aniso) {
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &caps.max_aniso);
		info("maximum texture anisotropy: %d\n", caps.max_aniso);
	} else {
		caps.max_aniso = 0;
	}

	if(caps.fbo) {
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &caps.max_rt);
		info("maximum render targets (fbo color attachments): %d\n", caps.max_rt);
	} else {
		caps.max_rt = 0;
	}
	glassert();

	return true;
}

static bool load_extensions()
{
	glewInit();
	return true;
}

static bool have_ext(const char *ext_str, const char *name)
{
	const char *ptr = strstr(ext_str, name);
	if(ptr && (ptr == ext_str || isspace(*(ptr - 1)))) {
		return true;
	}
	return false;
}
