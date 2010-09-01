#include "opengl.h"
#include "object.h"
#include "scene.h"

using namespace henge;

robject::robject()
{
	custom_render = 0;
}

robject::~robject()
{
}

robject *robject::clone() const
{
	robject *tmp = new robject(*this);
	return tmp;
}

void robject::apply_xform(int time)
{
	mesh.transform(get_xform_matrix(time));
}

void robject::set_material(const material &mat)
{
	this->mat = mat;
}

material *robject::get_material_ptr()
{
	return &mat;
}

const material &robject::get_material() const
{
	return mat;
}

trimesh *robject::get_mesh()
{
	return &mesh;
}

const trimesh *robject::get_mesh() const
{
	return &mesh;
}

aabox *robject::get_aabox() const
{
	bbox.min = mesh.get_aabb_min();
	bbox.max = mesh.get_aabb_max();
	return &bbox;
}

bsphere *robject::get_bsphere() const
{
	bsph.center = mesh.get_centroid();
	bsph.radius = mesh.get_bsph_radius();
	return &bsph;
}

void robject::set_render_func(void (*func)(const robject*, unsigned int, void*), void *cls)
{
	custom_render = func;
	cust_rend_cls = cls;
}

void robject::render(unsigned int msec) const
{
	if(custom_render) {
		custom_render(this, msec, cust_rend_cls);
		return;
	}

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	mult_matrix(get_xform_matrix(msec));

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT);

	mat.bind();
	mesh.draw();

	if(mat.get_shader()) {
		set_shader(0);
	}

	glPopAttrib();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

#define DRAW_ELEM(elem, c)			\
	glPushAttrib(GL_ENABLE_BIT);	\
	glDisable(GL_LIGHTING);			\
	glDisable(GL_TEXTURE_2D);		\
	glMatrixMode(GL_MODELVIEW);		\
	glPushMatrix();					\
	mult_matrix(get_xform_matrix(msec));	\
	mesh.draw_##elem(size, c);		\
	glMatrixMode(GL_MODELVIEW);		\
	glPopMatrix();					\
	glPopAttrib()


void robject::draw_vertices(float size, unsigned int msec) const
{
	DRAW_ELEM(vertices, color(1, 0, 0, 1));
}

void robject::draw_normals(float size, unsigned int msec) const
{
	DRAW_ELEM(normals, color(0.2, 1, 0.4, 1));
}

void robject::draw_tangents(float size, unsigned int msec) const
{
	DRAW_ELEM(tangents, color(0.2f, 0.4f, 1, 1));
}

namespace henge {
	extern unsigned int current_time;
}

// XXX not really efficient atm.
bool robject::operator <(const robject &rhs) const
{
	Matrix4x4 lhs_mat, rhs_mat;

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	mult_matrix(get_xform_matrix(current_time));
	store_matrix(&lhs_mat);
	glPopMatrix();

	glPushMatrix();
	mult_matrix(rhs.get_xform_matrix(current_time));
	store_matrix(&rhs_mat);
	glPopMatrix();

	Vector3 lhs_pos = mesh.get_centroid().transformed(lhs_mat);
	Vector3 rhs_pos = rhs.get_mesh()->get_centroid().transformed(rhs_mat);

	return lhs_pos.z < rhs_pos.z;
}
