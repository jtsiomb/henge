#ifndef HENGE_SKY_H_
#define HENGE_SKY_H_

#include "object.h"
#include "texture.h"
#include "sdr.h"
#include "colgrad.h"

namespace henge {

class SkyLayer;

class SkyDome {
private:
	std::vector<SkyLayer*> layers;
	Texture *sun_tex, *moon_tex;
	SphVector sun_pos, moon_pos;
	float sun_size, moon_size;

public:
	SkyDome();
	~SkyDome();
	bool add_layer(SkyLayer *layer);

	void set_sun_tex(Texture *tex);
	void set_moon_tex(Texture *tex);

	void set_sun_pos(const SphVector &svec);
	void set_moon_pos(const SphVector &svec);

	void set_sun_size(float sz);
	void set_moon_size(float sz);

	void draw(unsigned int msec = 0) const;

	void draw_sun() const;
	void draw_moon() const;
};


class SkyLayer {
protected:
	float radius;
	RObject *dome;
	Material *mat;
	Color col;
	bool enabled;

public:
	SkyLayer(float rad = 750.0f);
	virtual ~SkyLayer();

	void enable();
	void disable();
	bool is_enabled() const;

	void set_radius(float rad);
	float get_radius() const;

	void set_color(const Color &col);
	const Color &get_color() const;

	virtual void draw(unsigned int msec) const;
};

class SkyLayerGrad : public SkyLayer {
private:
	bool def_grad;
	ColorGradient grad;

public:
	SkyLayerGrad(float rad = 750.0f);
	SkyLayerGrad(const ColorGradient &grad);
	virtual ~SkyLayerGrad();

	bool load_grad(const char *fname);
	bool save_grad(const char *fname);
	void set_grad_color(float t, const Color &col);

	virtual void draw(unsigned int msec = 0) const;
};

class SkyLayerClouds : public SkyLayer {
protected:
	Vector3 scale, velocity;

public:
	SkyLayerClouds(float rad = 750.0f);
	virtual ~SkyLayerClouds();

	void set_scale(float scale);
	void set_scale(const Vector3 &scale);
	void set_velocity(const Vector2 &vel);

	virtual void draw(unsigned int msec) const = 0;
};

class SkyLayerCloudsTex : public SkyLayerClouds {
public:
	SkyLayerCloudsTex(float rad = 750.0f, Texture *tex = 0);
	virtual ~SkyLayerCloudsTex();

	void set_texture(Texture *tex);

	virtual void draw(unsigned int msec = 0) const;
};

class SkyLayerCloudsSdr : public SkyLayerClouds {
private:
	float anim_speed, coverage, sharpness;

public:
	SkyLayerCloudsSdr(float rad = 750.0f, Shader *sdr = 0);
	virtual ~SkyLayerCloudsSdr();

	void set_shader(Shader *sdr);
	void set_anim_speed(float s);
	void set_coverage(float cov);
	void set_sharpness(float shrp);

	virtual void draw(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_SKY_H_
