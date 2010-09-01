#ifndef HENGE_AABB_H_
#define HENGE_AABB_H_

#include "vmath.h"

namespace henge {

class bvolume {
public:
	virtual ~bvolume();

	virtual bool contains(const Vector3 &pt) const = 0;
	virtual bool intersect(const Ray &ray) const = 0;
};

class bsphere : public bvolume {
public:
	Vector3 center;
	float radius;

	bsphere();
	bsphere(const Vector3 &center, float radius);
	virtual ~bsphere();

	virtual bool contains(const Vector3 &pt) const;
	virtual bool intersect(const Ray &ray) const;
};

class aabox : public bvolume {
public:
	Vector3 min, max;

	aabox();
	aabox(const Vector3 &min, const Vector3 &max);
	virtual ~aabox();

	virtual bool contains(const Vector3 &pt) const;
	virtual bool intersect(const Ray &ray) const;
};

}	// namespace henge

#endif	// HENGE_AABB_H_
