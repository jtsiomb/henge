#ifndef RENDERER_H_
#define RENDERER_H_

#include "scene.h"

namespace henge {

class Renderer;

void set_renderer(Renderer *rend);
Renderer *get_renderer();

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

class Renderer {
protected:
	unsigned int rend_mask;

public:
	Renderer();
	virtual ~Renderer();

	void set_render_mask(unsigned int rmask);
	unsigned int get_render_mask() const;

	virtual void render(const Scene *scn, unsigned int msec = 0) const = 0;
};

class StdRenderer : public Renderer {
public:
	//virtual ~StdRenderer();
	virtual void render(const Scene *scn, unsigned int msec = 0) const;
};

// defined in deferred.cc
class DeferredRenderer : public Renderer {
protected:
	Shader *def_obj_sdr;
	Shader *point_light_sdr, *spot_light_sdr;
	Shader *dbg_sdr;

	bool init();
	void shutdown();

public:
	DeferredRenderer();
	virtual ~DeferredRenderer();

	virtual void render(const Scene *scn, unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// RENDERER_H_
