#ifndef HENGE_MESH_H_
#define HENGE_MESH_H_

#include "vmath.h"
#include "color.h"
#include "kdtree.h"

namespace henge {

enum {
	EL_VERTEX,
	EL_NORMAL,
	EL_TANGENT,
	EL_TEXCOORD,
	EL_COLOR,
	EL_INDEX,

	EL_COUNT
};

class TriMesh {
private:
	Vector3 *vert;
	Vector3 *norm;
	Vector3 *tang;
	Vector2 *tc;
	Vector4 *col;
	unsigned int *index;
	int nvert, nnorm, ntang, ntc, ncol, nindex;

	bool dynamic;

	mutable unsigned int dlist;
	mutable unsigned int vbo[EL_COUNT];
	mutable bool vbo_valid[EL_COUNT];

	KDTree<int> kdt;
	bool kdt_valid;

	Vector3 centroid;
	Vector3 aabb_min, aabb_max;
	float bsph_rad;
	bool bounds_valid;

	void build_kdtree();
	void setup_vertex_arrays() const;

	void calc_bounds();

	void init();

	void invalidate(int elmask);

public:
	TriMesh();
	~TriMesh();

	TriMesh(const TriMesh &mesh);
	TriMesh &operator =(const TriMesh &mesh);

	void set_dynamic(bool dynamic);
	bool get_dynamic() const;

	bool merge(const TriMesh &mesh);

	/* The data pointer can be null, in which no copy is attempted.
	 * Memory allocation and internal state setup is still performed,
	 * so that afterwards data may be filled in through the pointers
	 * returned by get_data_* below.
	 */
	bool set_data(int elem, const Vector4 *data, int count);
	bool set_data(int elem, const Vector3 *data, int count);
	bool set_data(int elem, const Vector2 *data, int count);
	bool set_data(int elem, const unsigned int *data, int count);

	Vector4 *get_data_vec4(int elem);
	const Vector4 *get_data_vec4(int elem) const;

	Vector3 *get_data_vec3(int elem);
	const Vector3 *get_data_vec3(int elem) const;

	Vector2 *get_data_vec2(int elem);
	const Vector2 *get_data_vec2(int elem) const;

	unsigned int *get_data_int(int elem);
	const unsigned int *get_data_int(int elem) const;

	int get_count(int elem) const;

	void calc_normals();

	void indexify(float threshold = 0.0001);

	void flip_winding();
	void flip_normals();
	void transform(const Matrix4x4 &mat);

	Vector3 get_centroid() const;
	Vector3 get_aabb_min() const;
	Vector3 get_aabb_max() const;
	// bounding sphere radius around the centroid (get_centroid)
	float get_bsph_radius() const;

	void draw() const;
	void draw_normals(float sz = 1.0, const Color &col = Color(0, 1, 0, 1)) const;
	void draw_tangents(float sz = 1.0, const Color &col = Color(0, 1, 0, 1)) const;
	void draw_vertices(float sz = 1.0, const Color &col = Color(1, 0, 0, 1)) const;

	bool intersect(const Ray &ray, float *pt = 0) const;
};

}	// namespace henge

#endif	// HENGE_MESH_H_
