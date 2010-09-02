#ifndef HENGE_POST_H_
#define HENGE_POST_H_

#include "texture.h"
#include "sdr.h"
#include "color.h"

namespace henge {

void overlay(const Texture *tex, const Color &col, const Shader *sdr = 0);

void overlay(const Vector2 &pos, const Vector2 &sz, const Color &col, const Texture *tex,
		const Shader *sdr = 0, unsigned int src_blend = GL_SRC_ALPHA,
		unsigned int dst_blend = GL_ONE_MINUS_SRC_ALPHA);

}	// namespace henge

#endif	// HENGE_POST_H_
