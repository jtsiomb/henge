#ifndef HENGE_COLOR_H_
#define HENGE_COLOR_H_

#include <vmath.h>
#include "int_types.h"
#include "byteorder.h"

namespace henge {

// our colors are just 4dimensional vectors
typedef Vector4 color;

inline uint32_t pack_color(int r, int g, int b, int a = 255)
{
	return ((r & 0xff) << RED_SHIFT) | ((g & 0xff) << GREEN_SHIFT) |
		((b & 0xff) << BLUE_SHIFT) | ((a & 0xff) << ALPHA_SHIFT);
}

inline uint32_t pack_color(const color &col)
{
	return pack_color((int)(col.x * 255.0f), (int)(col.y * 255.0f), (int)(col.z * 255.0f),
			(int)(col.w * 255.0f));
}

inline int unpack_red(uint32_t pix)
{
	return (pix >> RED_SHIFT) & 0xff;
}

inline int unpack_green(uint32_t pix)
{
	return (pix >> GREEN_SHIFT) & 0xff;
}

inline int unpack_blue(uint32_t pix)
{
	return (pix >> BLUE_SHIFT) & 0xff;
}

inline int unpack_alpha(uint32_t pix)
{
	return (pix >> ALPHA_SHIFT) & 0xff;
}

inline color unpack_color(uint32_t pix)
{
	return color((float)unpack_red(pix), (float)unpack_green(pix), (float)unpack_blue(pix),
			(float)unpack_alpha(pix));
}

}

#endif	// HENGE_COLOR_H_
