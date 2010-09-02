#ifndef HENGE_GEOM_H_
#define HENGE_GEOM_H_

#include "vmath.h"

namespace henge {

class Object {
protected:
	Vector3 pos;
	Matrix4x4 xform;

	mutable Vector3 cached_pos;
	mutable bool cache_valid;

public:
	Object();
	virtual ~Object();

	virtual void set_xform(const Matrix4x4 &mat);
	virtual const Matrix4x4 &get_xform() const;

	virtual void set_position(const Vector3 &vec);
	virtual Vector3 get_position(bool transformed = false) const;

	virtual bool raytest(const Ray &ray, Vector3 *pt) const = 0;
};

class Sphere : public Object {
protected:
	float radius;

public:
	Sphere(const Vector3 &pos = Vector3(0, 0, 0), float rad = 1.0f);
	virtual ~Sphere();

	void set_radius(float r);
	float get_radius() const;

	virtual bool raytest(const Ray &ray, Vector3 *pt) const;
};

class Plane : public Object {
protected:
	Vector3 norm;

public:
	Plane(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0));
	Plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
	virtual ~Plane();

	void set_normal(const Vector3 &norm);
	Vector3 get_normal() const;

	void set_points(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);

	virtual bool raytest(const Ray &ray, Vector3 *pt) const;
};

}	// namespace henge

#endif	// HENGE_GEOM_H_
