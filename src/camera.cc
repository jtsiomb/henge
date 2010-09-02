#include "opengl.h"
#include "camera.h"

using namespace henge;

Camera::~Camera() {}

Camera *Camera::clone() const
{
	printf("Camera::clone\n");
	return new Camera(*this);
}

Matrix4x4 Camera::get_matrix(unsigned int time) const
{
	return get_xform_matrix(time).inverse();
}

void Camera::bind(unsigned int time) const
{
	glMatrixMode(GL_MODELVIEW);

	load_matrix(get_matrix(time));
}


TargetCamera::TargetCamera()
{
	set_position(Vector3(0, 5, 10));
	set_target(Vector3(0, 0, 0));
	set_roll(0.0f);
}

TargetCamera::TargetCamera(const Vector3 &pos, const Vector3 &targ)
{
	set_position(pos);
	set_target(targ);
	set_roll(0.0f);
}

TargetCamera::~TargetCamera() {}

TargetCamera *TargetCamera::clone() const
{
	printf("TargetCamera::clone\n");
	return new TargetCamera(*this);
}

void TargetCamera::set_target(const Vector3 &pos, unsigned int time)
{
	target.set_position(pos, time);
}

Vector3 TargetCamera::get_target(unsigned int time) const
{
	return target.get_position(time);
}

void TargetCamera::set_roll(float roll, unsigned int time)
{
	// XXX piggyback on the unused scale track of the target
	target.set_scaling(Vector3(roll, roll, roll), time);
}

float TargetCamera::get_roll(unsigned int time) const
{
	// XXX piggyback on the unused scale track of the target
	return target.get_scaling(time).x;
}

Matrix4x4 TargetCamera::get_matrix(unsigned int time) const
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
