#ifndef HENGE_H_
#define HENGE_H_

#include "anim.h"
#include "bounds.h"
#include "byteorder.h"
#include "color.h"
#include "errlog.h"
#include "geom.h"
#include "henge.h"
#include "light.h"
#include "material.h"
#include "mesh.h"
#include "object.h"
#include "opengl.h"
#include "scene.h"
#include "renderer.h"
#include "sdr.h"
#include "unicache.h"
#include "texture.h"
#include "texenv.h"
#include "colgrad.h"
#include "cfgfile.h"
#include "post.h"
#include "sky.h"
#include "ggen.h"
#include "datapath.h"
#include "vmath/vmath.h"

namespace henge {

bool init();
bool init_no_gl();
void shutdown();

void set_viewport(int x, int y, int width, int height);
void get_viewport(int *x, int *y, int *width, int *height);

}

#endif	// HENGE_H_
