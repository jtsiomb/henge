#include "geom.h"

using namespace henge;

Object::Object()
{
	cache_valid = false;
}

Object::~Object() {}

void Object::set_xform(const Matrix4x4 &mat)
{
	xform = mat;
	cache_valid = false;
}

const Matrix4x4 &Object::get_xform() const
{
	return xform;
}

void Object::set_position(const Vector3 &vec)
{
	pos = vec;
	cache_valid = false;
}

Vector3 Object::get_position(bool transformed) const
{
	if(transformed) {
		cached_pos = pos.transformed(xform);
		cache_valid = true;
	}
	return pos;
}

// --- Sphere ---

Sphere::Sphere(const Vector3 &pos, float rad)
{
	this->pos = pos;
	radius = rad;
}

Sphere::~Sphere() {}


void Sphere::set_radius(float r)
{
	radius = r;
}

float Sphere::get_radius() const
{
	return radius;
}

bool Sphere::raytest(const Ray &ray, Vector3 *pt) const
{
	Vector3 pos = get_position(true);	// get transformed position

	float a = SQ(ray.dir.x) + SQ(ray.dir.y) + SQ(ray.dir.z);
	float b = 2.0 * ray.dir.x * (ray.origin.x - pos.x) +
				2.0 * ray.dir.y * (ray.origin.y - pos.y) +
				2.0 * ray.dir.z * (ray.origin.z - pos.z);
	float c = SQ(pos.x) + SQ(pos.y) + SQ(pos.z) +
				SQ(ray.origin.x) + SQ(ray.origin.y) + SQ(ray.origin.z) +
				2.0 * (-pos.x * ray.origin.x - pos.y * ray.origin.y - pos.z * ray.origin.z) - SQ(radius);

	float d = SQ(b) - 4.0 * a * c;
	if(d < 0.0) return false;

	float sqrt_d = sqrt(d);
	float t1 = (-b + sqrt_d) / (2.0 * a);
	float t2 = (-b - sqrt_d) / (2.0 * a);

	if((t1 < ERROR_MARGIN && t2 < ERROR_MARGIN) || (t1 > 1.0 && t2 > 1.0)) {
		return false;
	}

	if(pt) {
		if(t1 < ERROR_MARGIN) t1 = t2;
		if(t2 < ERROR_MARGIN) t2 = t1;

		float t = t1 < t2 ? t1 : t2;
		*pt = ray.origin + ray.dir * t;
	}
	return true;
}


// --- Plane ---

henge::Plane::Plane(const Vector3 &pos, const Vector3 &norm)
{
	this->pos = pos;
	this->norm = norm;
}

henge::Plane::Plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3)
{
	set_points(p1, p2, p3);
}

henge::Plane::~Plane() {}


void henge::Plane::set_normal(const Vector3 &norm)
{
	this->norm = norm;
}

Vector3 henge::Plane::get_normal() const
{
	return norm;
}

// define the plane by 3 points
void henge::Plane::set_points(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3)
{
	pos = p1;

	Vector3 v1 = p2 - p1;
	Vector3 v2 = p3 - p1;

	norm = cross_product(v1, v2).normalized();
}


bool henge::Plane::raytest(const Ray &ray, Vector3 *pt) const
{
	if(fabs(dot_product(norm, ray.dir)) < ERROR_MARGIN) {
		return false;
	}

	float t = -dot_product(norm, ray.origin - pos) / dot_product(norm, ray.dir);

	if(t < ERROR_MARGIN || t > 1.0) {
		return false;
	}

	*pt = ray.origin + ray.dir * t;
	return true;
}
