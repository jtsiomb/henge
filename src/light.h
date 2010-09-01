#ifndef HENGE_LIGHT_H_
#define HENGE_LIGHT_H_

#include "vmath.h"
#include "anim.h"
#include "color.h"

namespace henge {

class light : public xform_node {
protected:
	color ambient, diffuse, specular;
	float att[3];
	bool enabled;

public:
	light(const Vector3 &pos = Vector3(0, 0, 0), const color &col = color(1, 1, 1, 1));
	virtual ~light();

	virtual light *clone() const;

	virtual void set_ambient(const color &col);
	virtual color get_ambient() const;

	virtual void set_diffuse(const color &col);
	virtual color get_diffuse() const;

	virtual void set_specular(const color &col);
	virtual color get_specular() const;

	virtual void set_attenuation(float con, float lin, float quad);
	virtual void get_attenuation(float *con, float *lin, float *quad) const;

	virtual void enable();
	virtual void disable();
	virtual bool is_enabled() const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

class dirlight_base {
protected:
	Vector3 dir;

public:
	virtual ~dirlight_base();

	virtual dirlight_base *clone() const;

	virtual void set_direction(const Vector3 &dir);
	virtual const Vector3 &get_direction() const;
};

class spotlight : public light, public dirlight_base {
protected:
	float inner, outer, exponent;

public:
	spotlight();
	virtual ~spotlight();

	virtual spotlight *clone() const;

	virtual void set_cone(float inner, float outer);
	virtual void get_cone(float *inner, float *outer) const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

class dirlight : public light, public dirlight_base {
protected:
	Vector3 dir;

public:
	dirlight(const Vector3 &dir = Vector3(0, 0, -1));
	virtual ~dirlight();

	virtual dirlight *clone() const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_LIGHT_H_
