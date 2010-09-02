#include <cstdio>
#include "opengl.h"
#include "light.h"
#include "unicache.h"

using namespace henge;

Light::Light(const Vector3 &pos, const Color &col)
{
	set_position(pos);

	ambient = Color(0, 0, 0, 1);
	diffuse = col;
	specular = col;
	att[0] = 1.0f;
	att[1] = att[2] = 0.0f;

	enabled = true;
}

Light::~Light() {}

Light *Light::clone() const
{
	printf("Light::clone\n");
	return new Light(*this);
}

void Light::set_ambient(const Color &col)
{
	ambient = col;
}

Color Light::get_ambient() const
{
	return ambient;
}

void Light::set_diffuse(const Color &col)
{
	diffuse = col;
}

Color Light::get_diffuse() const
{
	return diffuse;
}

void Light::set_specular(const Color &col)
{
	specular = col;
}

Color Light::get_specular() const
{
	return specular;
}

void Light::set_attenuation(float con, float lin, float quad)
{
	att[0] = con;
	att[1] = lin;
	att[2] = quad;
}

void Light::get_attenuation(float *con, float *lin, float *quad) const
{
	if(con) *con = att[0];
	if(lin) *lin = att[1];
	if(quad) *quad = att[2];
}

void Light::enable()
{
	enabled = true;
}

void Light::disable()
{
	enabled = false;
}

bool Light::is_enabled() const
{
	return enabled;
}

#ifdef SINGLE_PRECISION_MATH
#define gl_lightv(lt, attr, v)	glLightfv(lt, attr, fvec)
#else
#define gl_lightv(lt, attr, v)	\
	do { \
		float fvec[] = {v.x, v.y, v.z, v.w}; \
		glLightfv(lt, attr, fvec); \
	} while(0)
#endif

bool Light::bind(int idx, unsigned int msec) const
{
	if(!enabled) return false;

	unsigned int lt = GL_LIGHT0 + idx;

	glEnable(lt);

	Vector4 pos = get_position(msec);
	Vector4 amb = ambient;
	Vector4 diff = diffuse;
	Vector4 spec = specular;

	gl_lightv(lt, GL_POSITION, pos);
	gl_lightv(lt, GL_AMBIENT, amb);
	gl_lightv(lt, GL_DIFFUSE, diff);
	gl_lightv(lt, GL_SPECULAR, spec);
	glLightf(lt, GL_CONSTANT_ATTENUATION, att[0]);
	glLightf(lt, GL_LINEAR_ATTENUATION, att[1]);
	glLightf(lt, GL_QUADRATIC_ATTENUATION, att[2]);
	glLightf(lt, GL_SPOT_CUTOFF, 180.0f);
	return true;
}

DirLightBase::~DirLightBase() {}

DirLightBase *DirLightBase::clone() const
{
	printf("DirLightBase::clone\n");
	return new DirLightBase(*this);
}

void DirLightBase::set_direction(const Vector3 &dir)
{
	this->dir = dir;
}

const Vector3 &DirLightBase::get_direction() const
{
	return dir;
}


SpotLight::SpotLight()
{
	dir = Vector3(0, 0, -1);
	inner = outer = 45;
	exponent = 0;
}

SpotLight::~SpotLight() {}

SpotLight *SpotLight::clone() const
{
	printf("SpotLight::clone\n");
	return new SpotLight(*this);
}

void SpotLight::set_cone(float inner, float outer)
{
	this->inner = inner;
	this->outer = outer;
}

void SpotLight::get_cone(float *inner, float *outer) const
{
	if(inner) *inner = this->inner;
	if(outer) *outer = this->outer;
}

bool SpotLight::bind(int idx, unsigned int msec) const
{
	if(!Light::bind(idx, msec)) {
		return false;
	}

	Quaternion q = get_rotation(msec);
	Quaternion p(0.0f, dir);
	p = q.inverse() * p * q;

	float sdir[] = {p.v.x, p.v.y, p.v.z, 1.0};

	unsigned int lt = GL_LIGHT0 + idx;
	glLightfv(lt, GL_SPOT_DIRECTION, sdir);
	glLightf(lt, GL_SPOT_EXPONENT, exponent);
	glLightf(lt, GL_SPOT_CUTOFF, outer);

	char var_name[32];

	sprintf(var_name, "uc_spot_inner[%d]", idx);
	cache_uniform(var_name, (float)DEG_TO_RAD(inner));
	sprintf(var_name, "uc_spot_inner%d", idx);
	cache_uniform(var_name, (float)DEG_TO_RAD(inner));

	sprintf(var_name, "uc_spot_outer[%d]", idx);
	cache_uniform(var_name, (float)DEG_TO_RAD(outer));
	sprintf(var_name, "uc_spot_outer%d", idx);
	cache_uniform(var_name, (float)DEG_TO_RAD(outer));

	return true;
}

DirLight::DirLight(const Vector3 &dir)
{
	this->dir = dir;
}

DirLight::~DirLight() {}

DirLight *DirLight::clone() const
{
	printf("DirLight::clone\n");
	return new DirLight(*this);
}

bool DirLight::bind(int idx, unsigned int msec) const
{
	if(!Light::bind(idx, msec)) {
		return false;
	}

	Vector3 ldir = dir.transformed(get_rotation(msec));
	float lpos[] = {-ldir.x, -ldir.y, -ldir.z, 0.0f};

	glLightfv(GL_LIGHT0 + idx, GL_POSITION, lpos);
	return true;
}
