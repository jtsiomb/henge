#ifndef HENGE_BYTEORDER_H_
#define HENGE_BYTEORDER_H_

#if !defined(HENGE_LENDIAN) && !defined(HENGE_BENDIAN)

#ifdef __BYTE_ORDER
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define HENGE_LENDIAN
#  else
#    define HENGE_BENDIAN
#  endif
#else
// default to little endian
#  define HENGE_LENDIAN
#endif	// __BYTE_ORDER

#endif	// !defined(HENGE_LENDIAN) && !defined(HENGE_BENDIAN)


// pixel component shift ammounts
#ifdef HENGE_LENDIAN

#define ALPHA_SHIFT	24
#define RED_SHIFT	16
#define GREEN_SHIFT	8
#define BLUE_SHIFT	0
#define GLPIXFMT	GL_BGRA

#else	// HENGE_BENDIAN

#define ALPHA_SHIFT	24
#define BLUE_SHIFT	16
#define GREEN_SHIFT	8
#define RED_SHIFT	0
#define GLPIXFMT	GL_RGBA

#endif	// shift ammounts


// define GL_BGRA if needed
#ifndef GL_BGRA
#define GL_BGRA		0x80E1
#endif

#endif	// HENGE_BYTEORDER_H_
