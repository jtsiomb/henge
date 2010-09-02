#ifndef HENGE_OBJECT_H_
#define HENGE_OBJECT_H_

#include "material.h"
#include "anim.h"
#include "mesh.h"
#include "bounds.h"

namespace henge {

class RObject : public XFormNode {
protected:
	TriMesh mesh;
	Material mat;

	mutable AABox bbox;
	mutable BSphere bsph;

	// custom rendering function
	void (*custom_render)(const RObject*, unsigned int, void*);
	void *cust_rend_cls;

public:

	RObject();
	virtual ~RObject();

	virtual RObject *clone() const;

	void apply_xform(int time = 0);

	void set_material(const Material &mat);
	const Material &get_material() const;
	Material *get_material_ptr();

	TriMesh *get_mesh();
	const TriMesh *get_mesh() const;

	AABox *get_aabox() const;
	BSphere *get_bsphere() const;

	void set_render_func(void (*func)(const RObject*, unsigned int, void*), void *cls);

	void render(unsigned int msec = 0) const;

	void draw_vertices(float size = 1.0f, unsigned int msec = 0) const;
	void draw_normals(float size = 1.0f, unsigned int msec = 0) const;
	void draw_tangents(float size = 1.0f, unsigned int msec = 0) const;

	// Z-sorting comparison operator
	bool operator <(const RObject &rhs) const;
};

}	// namespace henge

#endif	// HENGE_OBJECT_H_
