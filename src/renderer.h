#ifndef RENDERER_H_
#define RENDERER_H_

#include "scene.h"

namespace henge {

class renderer;

void set_renderer(renderer *rend);
renderer *get_renderer();

enum {
	REND_OBJ		= 1 << SCITEM_OBJ,
	REND_CAM		= 1 << SCITEM_CAM,
	REND_LIGHTS		= 1 << SCITEM_LIGHT,
	REND_PSYS		= 1 << SCITEM_PSYS,
	REND_RFUNCS		= 1 << SCITEM_RFUNC,

	REND_TRANSPARENT	= 1 << 10,

	REND_ALL		= 0xff
};

extern unsigned int current_time;

class renderer {
protected:
	unsigned int rend_mask;

public:
	renderer();
	virtual ~renderer();

	void set_render_mask(unsigned int rmask);
	unsigned int get_render_mask() const;

	virtual void render(const scene *scn, unsigned int msec = 0) const = 0;
};

class std_renderer : public renderer {
public:
	//virtual ~std_renderer();
	virtual void render(const scene *scn, unsigned int msec = 0) const;
};

// defined in deferred.cc
class deferred_renderer : public renderer {
protected:
	shader *def_obj_sdr;
	shader *point_light_sdr, *spot_light_sdr;
	shader *dbg_sdr;

	bool init();
	void shutdown();

public:
	deferred_renderer();
	virtual ~deferred_renderer();

	virtual void render(const scene *scn, unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// RENDERER_H_
