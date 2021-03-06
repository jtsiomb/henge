#ifndef HENGE_ANIM_H_
#define HENGE_ANIM_H_

#include <vector>
#include <algorithm>
#include "vmath.h"

namespace henge {

enum Interpolator {
	INTERP_STEP,
	INTERP_LINEAR,
	INTERP_CUBIC
};

enum Extrapolator {
	EXTRAP_CLAMP,
	EXTRAP_REPEAT,
	EXTRAP_PINGPONG		// TODO not implemented
};

// keyframe class
template <typename T>
struct TrackKey {
	T val;
	int time;

	TrackKey();
	TrackKey(const T &v, int t = 0);
	TrackKey(const T &v, float t = 0.0f);
	bool operator ==(const TrackKey &k) const;
	bool operator <(const TrackKey &k) const;
};

// track containing a number of keys
template <typename T>
class Track {
private:
	T def_val;
	std::vector<TrackKey<T> > keys;
	Interpolator interp;
	Extrapolator extrap;

	TrackKey<T> *get_nearest_key(int time);
	TrackKey<T> *get_nearest_key(int start, int end, int time);
	void get_key_interval(int time, const TrackKey<T> **start, const TrackKey<T> **end) const;

public:

	Track();

	void reset(const T &val);

	void set_interpolator(Interpolator interp);
	Interpolator get_interpolator() const;

	void set_extrapolator(Extrapolator extrap);
	Extrapolator get_extrapolator() const;

	void add_key(const TrackKey<T> &key);
	TrackKey<T> *get_key(int time);
	void delete_key(int time);

	int get_key_count() const;
	const TrackKey<T> &get_key_at(int idx) const;

	T operator()(int time) const;
	T operator()(float t) const;
};

class XFormNode {
private:
	char *name;

	Track<Vector3> ptrack, strack;
	Track<Quaternion> rtrack;

	Vector3 pivot;

	XFormNode *parent;
	std::vector<XFormNode*> children;

	mutable Matrix4x4 cache_matrix, cache_inv_matrix;
	mutable int cache_time;

	void invalidate_matrix_cache();

public:
	XFormNode();
	XFormNode(const XFormNode &node);
	virtual XFormNode &operator =(const XFormNode &node);
	virtual ~XFormNode();

	virtual XFormNode *clone() const;

	virtual void set_interpolator(Interpolator interp);
	virtual void set_extrapolator(Extrapolator extrap);

	virtual void set_name(const char *name);
	virtual const char *get_name() const;

	virtual void add_child(XFormNode *child);
	virtual void remove_child(XFormNode *child);

	virtual XFormNode **get_children();
	virtual int get_children_count() const;

	virtual void set_pivot(const Vector3 &p);
	virtual const Vector3 &get_pivot() const;

	virtual void reset_position();
	virtual void reset_rotation();
	virtual void reset_scaling();
	virtual void reset_xform();

	virtual void set_position(const Vector3 &pos, int time = 0);
	virtual void set_rotation(const Quaternion &rot, int time = 0);
	virtual void set_rotation(const Vector3 &euler, int time = 0);
	virtual void set_rotation(double angle, const Vector3 &axis, int time = 0);
	virtual void set_scaling(const Vector3 &s, int time = 0);

	virtual void translate(const Vector3 &pos, int time = 0);
	virtual void rotate(const Quaternion &rot, int time = 0);
	virtual void rotate(const Vector3 &euler, int time = 0);
	virtual void rotate(double angle, const Vector3 &axis, int time = 0);
	virtual void scale(const Vector3 &s, int time = 0);

	virtual Vector3 get_position(int time = 0) const;
	virtual Quaternion get_rotation(int time = 0) const;
	virtual Vector3 get_scaling(int time = 0) const;

	virtual Vector3 get_local_position(int time = 0) const;
	virtual Quaternion get_local_rotation(int time = 0) const;
	virtual Vector3 get_local_scaling(int time = 0) const;

	virtual Matrix4x4 get_xform_matrix(int time = 0) const;
	virtual Matrix4x4 get_inv_xform_matrix(int time = 0) const;
	virtual Matrix3x3 get_rot_matrix(int time = 0) const;
};

#include "anim.inl"

}

#endif	// HENGE_ANIM_H_
