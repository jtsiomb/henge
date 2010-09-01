#include <cstdio>
#include "opengl.h"
#include "light.h"
#include "unicache.h"

using namespace henge;

light::light(const Vector3 &pos, const color &col)
{
	set_position(pos);

	ambient = color(0, 0, 0, 1);
	diffuse = col;
	specular = col;
	att[0] = 1.0f;
	att[1] = att[2] = 0.0f;

	enabled = true;
}

light::~light() {}

light *light::clone() const
{
	printf("light::clone\n");
	return new light(*this);
}

void light::set_ambient(const color &col)
{
	ambient = col;
}

color light::get_ambient() const
{
	return ambient;
}

void light::set_diffuse(const color &col)
{
	diffuse = col;
}

color light::get_diffuse() const
{
	return diffuse;
}

void light::set_specular(const color &col)
{
	specular = col;
}

color light::get_specular() const
{
	return specular;
}

void light::set_attenuation(float con, float lin, float quad)
{
	att[0] = con;
	att[1] = lin;
	att[2] = quad;
}

void light::get_attenuation(float *con, float *lin, float *quad) const
{
	if(con) *con = att[0];
	if(lin) *lin = att[1];
	if(quad) *quad = att[2];
}

void light::enable()
{
	enabled = true;
}

void light::disable()
{
	enabled = false;
}

bool light::is_enabled() const
{
	return enabled;
}

bool light::bind(int idx, unsigned int msec) const
{
	if(!enabled) return false;

	unsigned int lt = GL_LIGHT0 + idx;

	glEnable(lt);

	Vector4 pos = get_position(msec);
	Vector4 amb = ambient;
	Vector4 diff = diffuse;
	Vector4 spec = specular;

	glLightfv(lt, GL_POSITION, (float*)&pos);
	glLightfv(lt, GL_AMBIENT, (float*)&amb);
	glLightfv(lt, GL_DIFFUSE, (float*)&diff);
	glLightfv(lt, GL_SPECULAR, (float*)&spec);
	glLightf(lt, GL_CONSTANT_ATTENUATION, att[0]);
	glLightf(lt, GL_LINEAR_ATTENUATION, att[1]);
	glLightf(lt, GL_QUADRATIC_ATTENUATION, att[2]);
	glLightf(lt, GL_SPOT_CUTOFF, 180.0f);
	return true;
}

dirlight_base::~dirlight_base() {}

dirlight_base *dirlight_base::clone() const
{
	printf("dirlight_base::clone\n");
	return new dirlight_base(*this);
}

void dirlight_base::set_direction(const Vector3 &dir)
{
	this->dir = dir;
}

const Vector3 &dirlight_base::get_direction() const
{
	return dir;
}


spotlight::spotlight()
{
	dir = Vector3(0, 0, -1);
	inner = outer = 45;
	exponent = 0;
}

spotlight::~spotlight() {}

spotlight *spotlight::clone() const
{
	printf("spotlight::clone\n");
	return new spotlight(*this);
}

void spotlight::set_cone(float inner, float outer)
{
	this->inner = inner;
	this->outer = outer;
}

void spotlight::get_cone(float *inner, float *outer) const
{
	if(inner) *inner = this->inner;
	if(outer) *outer = this->outer;
}

bool spotlight::bind(int idx, unsigned int msec) const
{
	if(!light::bind(idx, msec)) {
		return false;
	}

	Quaternion q = get_rotation(msec);
	Quaternion p(0.0f, dir);
	p = q.inverse() * p * q;

	Vector3 sdir = p.v;

	unsigned int lt = GL_LIGHT0 + idx;
	glLightfv(lt, GL_SPOT_DIRECTION, &sdir.x);
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

dirlight::dirlight(const Vector3 &dir)
{
	this->dir = dir;
}

dirlight::~dirlight() {}

dirlight *dirlight::clone() const
{
	printf("dirlight::clone\n");
	return new dirlight(*this);
}

bool dirlight::bind(int idx, unsigned int msec) const
{
	if(!light::bind(idx, msec)) {
		return false;
	}

	Vector3 ldir = dir.transformed(get_rotation(msec));
	float lpos[] = {-ldir.x, -ldir.y, -ldir.z, 0.0f};

	glLightfv(GL_LIGHT0 + idx, GL_POSITION, lpos);
	return true;
}
