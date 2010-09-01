#include "opengl.h"
#include "camera.h"

using namespace henge;

camera::~camera() {}

camera *camera::clone() const
{
	printf("camera::clone\n");
	return new camera(*this);
}

Matrix4x4 camera::get_matrix(unsigned int time) const
{
	return get_xform_matrix(time).inverse();
}

void camera::bind(unsigned int time) const
{
	glMatrixMode(GL_MODELVIEW);

	load_matrix(get_matrix(time));
}


target_camera::target_camera()
{
	set_position(Vector3(0, 5, 10));
	set_target(Vector3(0, 0, 0));
	set_roll(0.0f);
}

target_camera::target_camera(const Vector3 &pos, const Vector3 &targ)
{
	set_position(pos);
	set_target(targ);
	set_roll(0.0f);
}

target_camera::~target_camera() {}

target_camera *target_camera::clone() const
{
	printf("target_camera::clone\n");
	return new target_camera(*this);
}

void target_camera::set_target(const Vector3 &pos, unsigned int time)
{
	target.set_position(pos, time);
}

Vector3 target_camera::get_target(unsigned int time) const
{
	return target.get_position(time);
}

void target_camera::set_roll(float roll, unsigned int time)
{
	// XXX piggyback on the unused scale track of the target
	target.set_scaling(Vector3(roll, roll, roll), time);
}

float target_camera::get_roll(unsigned int time) const
{
	// XXX piggyback on the unused scale track of the target
	return target.get_scaling(time).x;
}

Matrix4x4 target_camera::get_matrix(unsigned int time) const
{
	Vector3 pos = get_position(time);
	Vector3 targ = target.get_position(time);
	
	Vector3 up(0, 1, 0);
	Vector3 dir = (targ - pos).normalized();
	Vector3 right = cross_product(dir, up).normalized();
	up = cross_product(right, dir);

	Quaternion q(dir, get_roll(time));
	up.transform(q);
	right.transform(q);

	dir = -dir;

	float tx = -dot_product(right, pos);
	float ty = -dot_product(up, pos);
	float tz = -dot_product(dir, pos);

	return Matrix4x4(right.x, right.y, right.z, tx,
			up.x, up.y, up.z, ty,
			dir.x, dir.y, dir.z, tz,
			0.0f, 0.0f, 0.0f, 1.0f);
}
