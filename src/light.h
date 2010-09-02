#ifndef HENGE_LIGHT_H_
#define HENGE_LIGHT_H_

#include "vmath.h"
#include "anim.h"
#include "color.h"

namespace henge {

class Light : public XFormNode {
protected:
	Color ambient, diffuse, specular;
	float att[3];
	bool enabled;

public:
	Light(const Vector3 &pos = Vector3(0, 0, 0), const Color &col = Color(1, 1, 1, 1));
	virtual ~Light();

	virtual Light *clone() const;

	virtual void set_ambient(const Color &col);
	virtual Color get_ambient() const;

	virtual void set_diffuse(const Color &col);
	virtual Color get_diffuse() const;

	virtual void set_specular(const Color &col);
	virtual Color get_specular() const;

	virtual void set_attenuation(float con, float lin, float quad);
	virtual void get_attenuation(float *con, float *lin, float *quad) const;

	virtual void enable();
	virtual void disable();
	virtual bool is_enabled() const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

class DirLightBase {
protected:
	Vector3 dir;

public:
	virtual ~DirLightBase();

	virtual DirLightBase *clone() const;

	virtual void set_direction(const Vector3 &dir);
	virtual const Vector3 &get_direction() const;
};

class SpotLight : public Light, public DirLightBase {
protected:
	float inner, outer, exponent;

public:
	SpotLight();
	virtual ~SpotLight();

	virtual SpotLight *clone() const;

	virtual void set_cone(float inner, float outer);
	virtual void get_cone(float *inner, float *outer) const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

class DirLight : public Light, public DirLightBase {
protected:
	Vector3 dir;

public:
	DirLight(const Vector3 &dir = Vector3(0, 0, -1));
	virtual ~DirLight();

	virtual DirLight *clone() const;

	virtual bool bind(int idx = 0, unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_LIGHT_H_
