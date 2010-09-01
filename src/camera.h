#ifndef HENGE_CAMERA_H_
#define HENGE_CAMERA_H_

#include "anim.h"
#include "vmath.h"

namespace henge {

class camera : public xform_node {
public:
	virtual ~camera();

	virtual camera *clone() const;

	virtual Matrix4x4 get_matrix(unsigned int time = 0) const;
	void bind(unsigned int time = 0) const;
};

class target_camera : public camera {
protected:
	xform_node target;

public:
	target_camera();
	target_camera(const Vector3 &pos, const Vector3 &targ);
	virtual ~target_camera();

	virtual target_camera *clone() const;

	void set_target(const Vector3 &pos, unsigned int time = 0);
	Vector3 get_target(unsigned int time = 0) const;

	void set_roll(float roll, unsigned int time = 0);
	float get_roll(unsigned int time = 0) const;

	virtual Matrix4x4 get_matrix(unsigned int time = 0) const;
};

}

#endif	// HENGE_CAMERA_H_
