#ifndef HENGE_GGEN_H_
#define HENGE_GGEN_H_

#include "mesh.h"

namespace henge {

enum TexCoordGen {
	TC_MAP_XZ,	// planar map on XZ plane (top)
	TC_MAP_XY,	// planar map on XY plane (front)
	TC_MAP_UV,	// planar map on the UV parameter plane
	TC_MAP_CUBE
};

// generation sample evaluation function (u, v, closure)
typedef float (*GenSampleFunc)(float, float, void*);

/* Generate surface of revolution by calling the supplied function to evaluate
 * the distance from the Y axis for each vertex.
 * Subdivision and range of samples along each parameter axis is controlled by
 * the usub, vsub, umax, vmax parameters.
 * Texture coordinates and tangent vectors are calculated based on the value of
 * tcgen.
 */
bool gen_revol(TriMesh *mesh, int usub, int vsub, float umax, float vmax,
		GenSampleFunc dist_func, void *dist_func_cls = 0,
		TexCoordGen tcgen = TC_MAP_UV);

/* Generate a heightfield by calling the supplied function to evaluate the
 * height from the XZ plane for each sample.
 * Subdivision is controlled by the usub, vsub parameters.
 * Texture coordinates and tangent vectors are calculated based on the value
 * of tcgen (I can't see how anything other than TC_MAP_XZ could make sense).
 */
bool gen_height(TriMesh *mesh, int usub, int vsub, GenSampleFunc height_func,
		void *height_func_cls = 0, TexCoordGen tcgen = TC_MAP_XZ);

// Generate a sphere. Calls gen_revol internally.
bool gen_sphere(TriMesh *mesh, float rad, int slices, int stacks, float umax = 1,
		float vmax = 1, TexCoordGen tcgen = TC_MAP_UV);

// Generate a cylinder. Calls gen_revol internally.
bool gen_cylinder(TriMesh *mesh, float rad, int slices, int stacks, float umax = 1,
		TexCoordGen tcgen = TC_MAP_UV);

bool gen_plane(TriMesh *mesh, int usub, int vsub, TexCoordGen tcgen = TC_MAP_XZ);

bool gen_box(TriMesh *mesh, float xsz, float ysz, float zsz, int xsub, int ysub, int zsub, TexCoordGen = TC_MAP_CUBE);

bool gen_icosa(TriMesh *mesh, TexCoordGen tcgen = TC_MAP_UV);
bool gen_geosphere(TriMesh *mesh, float rad, int subdiv, bool hemi = false, TexCoordGen tcgen = TC_MAP_UV);

bool gen_dodeca(TriMesh *mesh, TexCoordGen tcgen = TC_MAP_UV);

}	// namespace henge

#endif	// HENGE_GGEN_H_
