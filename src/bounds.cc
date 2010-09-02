#include "bounds.h"

using namespace henge;

BVolume::~BVolume() {}

BSphere::BSphere()
{
	radius = 0.0;
}

BSphere::BSphere(const Vector3 &center, float radius)
{
	this->center = center;
	this->radius = radius;
}

BSphere::~BSphere() {}

bool BSphere::contains(const Vector3 &pt) const
{
	return (pt - center).length_sq() < radius * radius;
}

bool BSphere::intersect(const Ray &ray) const
{
	float a = dot_product(ray.dir, ray.dir);
	float b = 2 * ray.dir.x * (ray.origin.x - center.x) +
		2 * ray.dir.y * (ray.origin.y - center.y) +
		2 * ray.dir.z * (ray.origin.z - center.z);
	float c = dot_product(center, center) + dot_product(ray.origin, ray.origin) +
		2 * dot_product(-center, ray.origin) - radius * radius;

	float discr = (b * b - 4.0 * a * c);
	if(discr < 0.0) {
		return false;
	}

	float sqrt_discr = sqrt(discr);
	float t1 = (-b + sqrt_discr) / (2.0 * a);
	float t2 = (-b - sqrt_discr) / (2.0 * a);

	if(t1 < ERROR_MARGIN) t1 = t2;
	if(t2 < ERROR_MARGIN) t2 = t1;

	float t = t1 < t2 ? t1 : t2;
	if(t < ERROR_MARGIN || t > 1.0) {
		return false;
	}
	return true;
}


AABox::AABox() {}

AABox::AABox(const Vector3 &min, const Vector3 &max)
{
	this->min = min;
	this->max = max;
}

AABox::~AABox() {}

bool AABox::contains(const Vector3 &pt) const
{
	return pt.x >= min.x && pt.y >= min.y && pt.z >= min.z &&
		pt.x < max.x && pt.y < max.y && pt.z < max.z;
}

/* ray-aabb intersection test based on:
 * "An Efficient and Robust Ray-Box Intersection Algorithm",
 * Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 * Journal of graphics tools, 10(1):49-54, 2005
 */
bool AABox::intersect(const Ray &ray) const
{
	Vector3 bbox[2] = {min, max};
	static const float t0 = 0.0;
	static const float t1 = 1.0;

	int xsign = (int)(ray.dir.x < 0.0);
	float invdirx = 1.0 / ray.dir.x;
	float tmin = (bbox[xsign].x - ray.origin.x) * invdirx;
	float tmax = (bbox[1 - xsign].x - ray.origin.x) * invdirx;

	int ysign = (int)(ray.dir.y < 0.0);
	float invdiry = 1.0 / ray.dir.y;
	float tymin = (bbox[ysign][1] - ray.origin.y) * invdiry;
	float tymax = (bbox[1 - ysign][1] - ray.origin.y) * invdiry;

	if((tmin > tymax) || (tymin > tmax)) {
		return false;
	}

	if(tymin > tmin) tmin = tymin;
	if(tymax < tmax) tmax = tymax;

	int zsign = (int)(ray.dir.z < 0.0);
	float invdirz = 1.0 / ray.dir.z;
	float tzmin = (bbox[zsign][2] - ray.origin.z) * invdirz;
	float tzmax = (bbox[1 - zsign][2] - ray.origin.z) * invdirz;

	if((tmin > tzmax) || (tzmin > tmax)) {
		return false;
	}

	if(tzmin > tmin) tmin = tzmin;
	if(tzmax < tmax) tmax = tzmax;

	return (tmin < t1) && (tmax > t0);
}
