#include <stdio.h>
#include <algorithm>
#include <climits>
#include "anim.h"

using namespace henge;
using namespace std;

#define INVAL_TIME		UINT_MAX

XFormNode::XFormNode()
{
	parent = 0;
	name = 0;
	cache_time = INVAL_TIME;

	reset_xform();
}

XFormNode::XFormNode(const XFormNode &node)
{
	*this = node;
}

XFormNode &XFormNode::operator =(const XFormNode &node)
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

XFormNode::~XFormNode()
{
	delete [] name;
}

void XFormNode::invalidate_matrix_cache()
{
	cache_time = INVAL_TIME;
}

XFormNode *XFormNode::clone() const
{
	printf("XFormNode::clone\n");
	return new XFormNode(*this);
}

void XFormNode::set_interpolator(Interpolator interp)
{
	ptrack.set_interpolator(interp);
	rtrack.set_interpolator(interp);
	strack.set_interpolator(interp);
	invalidate_matrix_cache();
}

void XFormNode::set_extrapolator(Extrapolator extrap)
{
	ptrack.set_extrapolator(extrap);
	rtrack.set_extrapolator(extrap);
	strack.set_extrapolator(extrap);
	invalidate_matrix_cache();
}

void XFormNode::set_name(const char *name)
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

const char *XFormNode::get_name() const
{
	return name;
}

void XFormNode::add_child(XFormNode *child)
{
	if(find(children.begin(), children.end(), child) == children.end()) {
		child->parent = this;
		child->invalidate_matrix_cache();
		children.push_back(child);
	}
}

void XFormNode::remove_child(XFormNode *child)
{
	vector<XFormNode*>::iterator iter;
	iter = find(children.begin(), children.end(), child);
	if(iter != children.end()) {
		(*iter)->invalidate_matrix_cache();
		children.erase(iter);
	}
}

XFormNode **XFormNode::get_children()
{
	return &children[0];
}

int XFormNode::get_children_count() const
{
	return (int)children.size();
}

void XFormNode::set_pivot(const Vector3 &p)
{
	pivot = p;
	invalidate_matrix_cache();
}

const Vector3 &XFormNode::get_pivot() const
{
	return pivot;
}

void XFormNode::reset_position()
{
	ptrack.reset(Vector3(0, 0, 0));
	invalidate_matrix_cache();
}

void XFormNode::reset_rotation()
{
	rtrack.reset(Quaternion());
	invalidate_matrix_cache();
}

void XFormNode::reset_scaling()
{
	strack.reset(Vector3(1, 1, 1));
	invalidate_matrix_cache();
}

void XFormNode::reset_xform()
{
	reset_position();
	reset_rotation();
	reset_scaling();
}

void XFormNode::set_position(const Vector3 &pos, int time)
{
	TrackKey<Vector3> *key = ptrack.get_key(time);
	if(key) {
		key->val = pos;
	} else {
		ptrack.add_key(TrackKey<Vector3>(pos, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

static void set_rotation_quat(Track<Quaternion> *track, const Quaternion &rot, int time)
{
	TrackKey<Quaternion> *key = track->get_key(time);
	if(key) {
		key->val = rot;
	} else {
		track->add_key(TrackKey<Quaternion>(rot, time));
	}
}

void XFormNode::set_rotation(const Quaternion &rot, int time)
{
	set_rotation_quat(&rtrack, rot, time);

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void XFormNode::set_rotation(const Vector3 &euler, int time)
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

void XFormNode::set_rotation(double angle, const Vector3 &axis, int time)
{
	set_rotation_quat(&rtrack, Quaternion(axis, angle), time);

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void XFormNode::set_scaling(const Vector3 &s, int time)
{
	TrackKey<Vector3> *key = strack.get_key(time);
	if(key) {
		key->val = s;
	} else {
		strack.add_key(TrackKey<Vector3>(s, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void XFormNode::translate(const Vector3 &pos, int time)
{
	TrackKey<Vector3> *key = ptrack.get_key(time);
	if(key) {
		key->val += pos;
	} else {
		ptrack.add_key(TrackKey<Vector3>(pos, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void XFormNode::rotate(const Quaternion &rot, int time)
{
	TrackKey<Quaternion> *key = rtrack.get_key(time);
	if(key) {
		key->val = rot * key->val;
	} else {
		rtrack.add_key(TrackKey<Quaternion>(rot, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

void XFormNode::rotate(const Vector3 &euler, int time)
{
	Quaternion xrot, yrot, zrot;
	xrot.set_rotation(Vector3(1, 0, 0), euler.x);
	yrot.set_rotation(Vector3(0, 1, 0), euler.y);
	zrot.set_rotation(Vector3(0, 0, 1), euler.z);
	rotate(xrot * yrot * zrot, time);
}

void XFormNode::rotate(double angle, const Vector3 &axis, int time)
{
	rotate(Quaternion(axis, angle), time);
}

void XFormNode::scale(const Vector3 &s, int time)
{
	TrackKey<Vector3> *key = strack.get_key(time);
	if(key) {
		key->val *= s;
	} else {
		strack.add_key(TrackKey<Vector3>(s, time));
	}

	if(time == cache_time) {
		invalidate_matrix_cache();
	}
}

Vector3 XFormNode::get_position(int time) const
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

Vector3 XFormNode::get_local_position(int time) const
{
	return ptrack(time);
}

Quaternion XFormNode::get_local_rotation(int time) const
{
	return rtrack(time);
}

Vector3 XFormNode::get_local_scaling(int time) const
{
	return strack(time);
}

Quaternion XFormNode::get_rotation(int time) const
{
	if(parent) {
		return parent->get_rotation(time) * rtrack(time);
	}
	return rtrack(time);
}

Vector3 XFormNode::get_scaling(int time) const
{
	if(parent) {
		return strack(time) * parent->get_scaling(time);
	}
	return strack(time);
}

Matrix4x4 XFormNode::get_xform_matrix(int time) const
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

Matrix4x4 XFormNode::get_inv_xform_matrix(int time) const
{
	get_xform_matrix(time);	// calculate invxform if needed
	return cache_matrix.inverse();//cache_inv_matrix;
}

Matrix3x3 XFormNode::get_rot_matrix(int time) const
{
	Quaternion rot = get_rotation(time);
	return rot.get_rotation_matrix();
}
