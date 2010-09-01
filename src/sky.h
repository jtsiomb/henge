#ifndef HENGE_SKY_H_
#define HENGE_SKY_H_

#include "object.h"
#include "texture.h"
#include "sdr.h"
#include "colgrad.h"

namespace henge {

class sky_layer;

class skydome {
private:
	std::vector<sky_layer*> layers;
	texture *sun_tex, *moon_tex;
	SphVector sun_pos, moon_pos;
	float sun_size, moon_size;

public:
	skydome();
	~skydome();
	bool add_layer(sky_layer *layer);

	void set_sun_tex(texture *tex);
	void set_moon_tex(texture *tex);

	void set_sun_pos(const SphVector &svec);
	void set_moon_pos(const SphVector &svec);

	void set_sun_size(float sz);
	void set_moon_size(float sz);

	void draw(unsigned int msec = 0) const;

	void draw_sun() const;
	void draw_moon() const;
};


class sky_layer {
protected:
	float radius;
	robject *dome;
	material *mat;
	color col;
	bool enabled;

public:
	sky_layer(float rad = 750.0f);
	virtual ~sky_layer();

	void enable();
	void disable();
	bool is_enabled() const;

	void set_radius(float rad);
	float get_radius() const;

	void set_color(const color &col);
	const color &get_color() const;

	virtual void draw(unsigned int msec) const;
};

class sky_layer_grad : public sky_layer {
private:
	bool def_grad;
	color_gradient grad;

public:
	sky_layer_grad(float rad = 750.0f);
	sky_layer_grad(const color_gradient &grad);
	virtual ~sky_layer_grad();

	bool load_grad(const char *fname);
	bool save_grad(const char *fname);
	void set_grad_color(float t, const color &col);

	virtual void draw(unsigned int msec = 0) const;
};

class sky_layer_clouds : public sky_layer {
protected:
	Vector3 scale, velocity;

public:
	sky_layer_clouds(float rad = 750.0f);
	virtual ~sky_layer_clouds();

	void set_scale(float scale);
	void set_scale(const Vector3 &scale);
	void set_velocity(const Vector2 &vel);

	virtual void draw(unsigned int msec) const = 0;
};

class sky_layer_clouds_tex : public sky_layer_clouds {
public:
	sky_layer_clouds_tex(float rad = 750.0f, texture *tex = 0);
	virtual ~sky_layer_clouds_tex();

	void set_texture(texture *tex);

	virtual void draw(unsigned int msec = 0) const;
};

class sky_layer_clouds_sdr : public sky_layer_clouds {
private:
	float anim_speed, coverage, sharpness;

public:
	sky_layer_clouds_sdr(float rad = 750.0f, shader *sdr = 0);
	virtual ~sky_layer_clouds_sdr();

	void set_shader(shader *sdr);
	void set_anim_speed(float s);
	void set_coverage(float cov);
	void set_sharpness(float shrp);

	virtual void draw(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_SKY_H_
