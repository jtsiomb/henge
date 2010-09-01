#ifndef HENGE_GEOM_H_
#define HENGE_GEOM_H_

#include "vmath.h"

namespace henge {

class object {
protected:
	Vector3 pos;
	Matrix4x4 xform;

	mutable Vector3 cached_pos;
	mutable bool cache_valid;

public:
	object();
	virtual ~object();

	virtual void set_xform(const Matrix4x4 &mat);
	virtual const Matrix4x4 &get_xform() const;

	virtual void set_position(const Vector3 &vec);
	virtual Vector3 get_position(bool transformed = false) const;

	virtual bool raytest(const Ray &ray, Vector3 *pt) const = 0;
};

class sphere : public object {
protected:
	float radius;

public:
	sphere(const Vector3 &pos = Vector3(0, 0, 0), float rad = 1.0f);
	virtual ~sphere();

	void set_radius(float r);
	float get_radius() const;

	virtual bool raytest(const Ray &ray, Vector3 *pt) const;
};

class plane : public object {
protected:
	Vector3 norm;

public:
	plane(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0));
	plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
	virtual ~plane();

	void set_normal(const Vector3 &norm);
	Vector3 get_normal() const;

	void set_points(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);

	virtual bool raytest(const Ray &ray, Vector3 *pt) const;
};

}	// namespace henge

#endif	// HENGE_GEOM_H_
