#include <stdio.h>
#include <algorithm>
#include <climits>
#include "anim.h"

using namespace henge;
using namespace std;

#define INVAL_TIME		UINT_MAX

xform_node::xform_node()
{
	parent = 0;
	name = 0;
	cache_time = INVAL_TIME;

	reset_xform();
}

xform_node::xform_node(const xform_node &node)
{
	*this = node;
}

xform_node &xform_node::operator =(const xform_node &node)
{
	if(this == &node) {
		return *this;
	}

	if(node.name) {
		name = new char[strlen(node.name) + 1];
		strcpy(name, node.name);
	} else {
		name = 0;
	}
	parent = node.parent;
	children = node.children;

	ptrack = node.ptrack;
	strack = node.strack;
	rtrack = node.rtrack;
	pivot = node.pivot;

	cache_matrix = node.cache_matrix;
	cache_inv_matrix = node.cache_inv_matrix;
	cache_time = node.cache_time;

	return *this;
}

xform_node::~xform_node()
{
	delete [] name;
}

void xform_node::invalidate_matrix_cache()
{
	cache_time = INVAL_TIME;
}

xform_node *xform_node::clone() const
{
	printf("xform_node::clone\n");
	return new xform_node(*this);
}

void xform_node::set_interpolator(interpolator interp)
{
	ptrack.set_interpolator(interp);
	rtrack.set_interpolator(interp);
	strack.set_interpolator(interp);
	invalidate_matrix_cache();
}

void xform_node::set_extrapolator(extrapolator extrap)
{
	ptrack.set_extrapolator(extrap);
	rtrack.set_extrapolator(extrap);
	strack.set_extrapolator(extrap);
	invalidate_matrix_cache();
}

void xform_node::set_name(const char *name)
{
	if(this->name) {
		delete [] this->name;
		this->name = 0;
	}

	if(name) {
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
	}
}

const char *xform_node::get_name() const
{
	return name;
}

void xform_node::add_child(xform_node *child)
{
	if(find(children.begin(), children.end(), child) == children.end()) {
		child->parent = this;
		child->invalidate_matrix_cache();
		children.push_back(child);
	}
}

void xform_node::remove_child(xform_node *child)
{
	vector<xform_node*>::iterator iter;
	iter = find(children.begin(), children.end(), child);
	if(iter != children.end()) {
		(*iter)->invalidate_matrix_cache();
		children.erase(iter);
	}
}

xform_node **xform_node::get_children()
{
	return &children[0];
}

int xform_node::get_children_count() const
{
	return (int)children.size();
}

void xform_node::set_pivot(const Vector3 &p)
{
	pivot = p;
	invalidate_matrix_cache();
}

const Vector3 &xform_node::get_pivot() const
{
	return pivot;
}

void xform_node::reset_position()
{
	ptrack.reset(Vector3(0, 0, 0));
	invalidate_matrix_cache();
}

void xform_node::reset_rotation()
{
	rtrack.reset(Quaternion());
	invalidate_matrix_cache();
}

void xform_node::reset_scaling()
{
	strack.reset(Vector3(1, 1, 1));
	invalidate_matrix_cache();
}

void xform_node::reset_xform()
{
	reset_position();
	reset_rotation();
	reset_scaling();
}
	
void xform_node::set_position(const Vector3 &pos, int time)
{
	track_key<Vector3> *key = ptrack.get_key(time);
	if(key) {
		key->val = pos;
	} else {
		ptrack.add_key(track_key<Vector3>(pos, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

static void set_rotation_quat(track<Quaternion> *track, const Quaternion &rot, int time)
{
	track_key<Quaternion> *key = track->get_key(time);
	if(key) {
		key->val = rot;
	} else {
		track->add_key(track_key<Quaternion>(rot, time));
	}
}

void xform_node::set_rotation(const Quaternion &rot, int time)
{
	set_rotation_quat(&rtrack, rot, time);

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void xform_node::set_rotation(const Vector3 &euler, int time)
{
	Quaternion xrot, yrot, zrot;
	xrot.set_rotation(Vector3(1, 0, 0), euler.x);
	yrot.set_rotation(Vector3(0, 1, 0), euler.y);
	zrot.set_rotation(Vector3(0, 0, 1), euler.z);
	set_rotation_quat(&rtrack, xrot * yrot * zrot, time);

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void xform_node::set_rotation(double angle, const Vector3 &axis, int time)
{
	set_rotation_quat(&rtrack, Quaternion(axis, angle), time);

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void xform_node::set_scaling(const Vector3 &s, int time)
{
	track_key<Vector3> *key = strack.get_key(time);
	if(key) {
		key->val = s;
	} else {
		strack.add_key(track_key<Vector3>(s, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}
	
void xform_node::translate(const Vector3 &pos, int time)
{
	track_key<Vector3> *key = ptrack.get_key(time);
	if(key) {
		key->val += pos;
	} else {
		ptrack.add_key(track_key<Vector3>(pos, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void xform_node::rotate(const Quaternion &rot, int time)
{
	track_key<Quaternion> *key = rtrack.get_key(time);
	if(key) {
		key->val = rot * key->val;
	} else {
		rtrack.add_key(track_key<Quaternion>(rot, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void xform_node::rotate(const Vector3 &euler, int time)
{
	Quaternion xrot, yrot, zrot;
	xrot.set_rotation(Vector3(1, 0, 0), euler.x);
	yrot.set_rotation(Vector3(0, 1, 0), euler.y);
	zrot.set_rotation(Vector3(0, 0, 1), euler.z);
	rotate(xrot * yrot * zrot, time);
}

void xform_node::rotate(double angle, const Vector3 &axis, int time)
{
	rotate(Quaternion(axis, angle), time);
}

void xform_node::scale(const Vector3 &s, int time)
{
	track_key<Vector3> *key = strack.get_key(time);
	if(key) {
		key->val *= s;
	} else {
		strack.add_key(track_key<Vector3>(s, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

Vector3 xform_node::get_position(int time) const
{
	Vector3 pos = ptrack(time);
	if(parent) {
		Vector3 parpos = parent->get_position(time);
		Quaternion parrot = get_rotation(time);

		pos -= parpos;
		pos.transform(parrot.conjugate());
		pos += parpos + parpos.transformed(parrot.conjugate());
		pos *= parent->get_scaling(time);
	}
	return pos;
}

Vector3 xform_node::get_local_position(int time) const
{
	return ptrack(time);
}

Quaternion xform_node::get_local_rotation(int time) const
{
	return rtrack(time);
}

Vector3 xform_node::get_local_scaling(int time) const
{
	return strack(time);
}

Quaternion xform_node::get_rotation(int time) const
{
	if(parent) {
		return parent->get_rotation(time) * rtrack(time);
	}
	return rtrack(time);
}

Vector3 xform_node::get_scaling(int time) const
{
	if(parent) {
		return strack(time) * parent->get_scaling(time);
	}
	return strack(time);
}

Matrix4x4 xform_node::get_xform_matrix(int time) const
{
	if(cache_time != time) {
		Matrix4x4 trans_mat, rot_mat, scale_mat, pivot_mat, neg_pivot_mat;
	
		Vector3 pos = ptrack(time);
		Quaternion rot = rtrack(time);
		Vector3 scale = strack(time);

		pivot_mat.set_translation(pivot);
		neg_pivot_mat.set_translation(-pivot);

		trans_mat.set_translation(pos);
		rot_mat = (Matrix4x4)rot.get_rotation_matrix();
		scale_mat.set_scaling(scale);

		cache_matrix = pivot_mat * trans_mat * rot_mat * scale_mat * neg_pivot_mat;
		if(parent) {
			cache_matrix = parent->get_xform_matrix(time) * cache_matrix;// * parent->get_xform_matrix(time);
		}

		// XXX probably won't need this bit (uncomment if it becomes useful)
		//cache_inv_matrix = cache_matrix.inverse();
		cache_time = time;
	}
	return cache_matrix;
}

Matrix4x4 xform_node::get_inv_xform_matrix(int time) const
{
	get_xform_matrix(time);	// calculate invxform if needed
	return cache_matrix.inverse();//cache_inv_matrix;
}

Matrix3x3 xform_node::get_rot_matrix(int time) const
{
	Quaternion rot = get_rotation(time);
	return rot.get_rotation_matrix();
}
