#ifndef HENGE_OBJECT_H_
#define HENGE_OBJECT_H_

#include "material.h"
#include "anim.h"
#include "mesh.h"
#include "bounds.h"

namespace henge {

class robject : public xform_node {
protected:
	trimesh mesh;
	material mat;

	mutable aabox bbox;
	mutable bsphere bsph;

	// custom rendering function
	void (*custom_render)(const robject*, unsigned int, void*);
	void *cust_rend_cls;

public:

	robject();
	virtual ~robject();

	virtual robject *clone() const;

	void apply_xform(int time = 0);

	void set_material(const material &mat);
	const material &get_material() const;
	material *get_material_ptr();

	trimesh *get_mesh();
	const trimesh *get_mesh() const;

	aabox *get_aabox() const;
	bsphere *get_bsphere() const;

	void set_render_func(void (*func)(const robject*, unsigned int, void*), void *cls);

	void render(unsigned int msec = 0) const;

	void draw_vertices(float size = 1.0f, unsigned int msec = 0) const;
	void draw_normals(float size = 1.0f, unsigned int msec = 0) const;
	void draw_tangents(float size = 1.0f, unsigned int msec = 0) const;

	// Z-sorting comparison operator
	bool operator <(const robject &rhs) const;
};

}	// namespace henge

#endif	// HENGE_OBJECT_H_
