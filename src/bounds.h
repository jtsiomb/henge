#ifndef HENGE_AABB_H_
#define HENGE_AABB_H_

#include "vmath.h"

namespace henge {

class BVolume {
public:
	virtual ~BVolume();

	virtual bool contains(const Vector3 &pt) const = 0;
	virtual bool intersect(const Ray &ray) const = 0;
};

class BSphere : public BVolume {
public:
	Vector3 center;
	float radius;

	BSphere();
	BSphere(const Vector3 &center, float radius);
	virtual ~BSphere();

	virtual bool contains(const Vector3 &pt) const;
	virtual bool intersect(const Ray &ray) const;
};

class AABox : public BVolume {
public:
	Vector3 min, max;

	AABox();
	AABox(const Vector3 &min, const Vector3 &max);
	virtual ~AABox();

	virtual bool contains(const Vector3 &pt) const;
	virtual bool intersect(const Ray &ray) const;
};

}	// namespace henge

#endif	// HENGE_AABB_H_
