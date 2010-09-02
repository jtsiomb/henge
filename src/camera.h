#ifndef HENGE_CAMERA_H_
#define HENGE_CAMERA_H_

#include "anim.h"
#include "vmath.h"

namespace henge {

class Camera : public XFormNode {
public:
	virtual ~Camera();

	virtual Camera *clone() const;

	virtual Matrix4x4 get_matrix(unsigned int time = 0) const;
	void bind(unsigned int time = 0) const;
};

class TargetCamera : public Camera {
protected:
	XFormNode target;

public:
	TargetCamera();
	TargetCamera(const Vector3 &pos, const Vector3 &targ);
	virtual ~TargetCamera();

	virtual TargetCamera *clone() const;

	void set_target(const Vector3 &pos, unsigned int time = 0);
	Vector3 get_target(unsigned int time = 0) const;

	void set_roll(float roll, unsigned int time = 0);
	float get_roll(unsigned int time = 0) const;

	virtual Matrix4x4 get_matrix(unsigned int time = 0) const;
};

}

#endif	// HENGE_CAMERA_H_
