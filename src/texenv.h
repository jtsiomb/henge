#ifndef TEXENV_H_
#define TEXENV_H_

namespace henge {

/* operators:
 * - GL_REPLACE
 * - GL_ADD
 * - GL_SUB
 * - GL_MODULATE
 * - ... more ...
 */

/* arguments:
 * - GL_PREVIOUS
 * - GL_PRIMARY_COLOR
 * - GL_TEXTURE
 * - ... more ...
 */

void set_texenv_color(int tunit, int op, int arg1, int arg2, int arg3 = -1);
void set_texenv_alpha(int tunit, int op, int arg1, int arg2, int arg3 = -1);

void set_default_texenv(int tunit = -1);

}

#endif	// TEXENV_H_
