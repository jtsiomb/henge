#include "texenv.h"
#include "opengl.h"

using namespace henge;

enum { COLOR, ALPHA };
static inline void set_texenv(int tunit, int op, int arg1, int arg2, int arg3, int elem);

void henge::set_texenv_color(int tunit, int op, int arg1, int arg2, int arg3)
{
	set_texenv(tunit, op, arg1, arg2, arg3, COLOR);
}

void henge::set_texenv_alpha(int tunit, int op, int arg1, int arg2, int arg3)
{
	set_texenv(tunit, op, arg1, arg2, arg3, ALPHA);
}

void henge::set_default_texenv(int tunit)
{
	if(tunit < 0) {
		for(int i=0; i<caps.max_tex_units; i++) {
			set_default_texenv(i);
		}
	} else {
		set_texenv_color(tunit, GL_MODULATE, GL_PREVIOUS, GL_TEXTURE);
		set_texenv_alpha(tunit, GL_MODULATE, GL_PREVIOUS, GL_TEXTURE);
	}
}

static inline void set_texenv(int tunit, int op, int arg1, int arg2, int arg3, int elem)
{
	if(!caps.multitex) return;

	// GL_COMBINE_RGB_ARB + 1 == GL_COBMBINE_ALPHA_ARB
	int comb = GL_COMBINE_RGB_ARB + elem;
	// GL_SOURCE0_RGB_ARB + 8 == GL_SOURCE0_ALPHA_ARB (etc...)
	int src0 = GL_SOURCE0_RGB_ARB + elem * 8;
	int src1 = GL_SOURCE1_RGB_ARB + elem * 8;

	glActiveTextureARB(GL_TEXTURE0_ARB + tunit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, comb, op);
	glTexEnvi(GL_TEXTURE_ENV, src0, arg1);
	glTexEnvi(GL_TEXTURE_ENV, src1, arg2);
	if(arg3 != -1) {
		int src2 = GL_SOURCE2_RGB_ARB + elem * 3;
		glTexEnvi(GL_TEXTURE_ENV, src2, arg3);
	}

	if(tunit != 0) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
}
