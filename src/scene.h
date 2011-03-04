#ifndef HENGE_SCENE_H_
#define HENGE_SCENE_H_

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "object.h"
#include "light.h"
#include "camera.h"
#include "psys.h"
#include "renderfunc.h"
#include "bounds.h"

namespace henge {

enum {
	SCITEM_OBJ,
	SCITEM_CAM,
	SCITEM_LIGHT,
	SCITEM_PSYS,
	SCITEM_RFUNC,

	NUM_SCITEMS
};

class Scene {
protected:
	std::map<std::string, RObject*> objmap;

	std::vector<RObject*> objects;
	std::vector<Light*> lights;
	std::vector<Camera*> cameras;
	std::vector<ParticleSystem*> particles;
	std::vector<RenderFunc> rfuncs;

	mutable AABox bbox;
	mutable BSphere bsph;
	mutable bool bounds_valid;

	// maps each item (object/light/etc) to a flag controlling
	// automatic deletion of the item when clean is called.
	std::map<const void*, bool> del_item;

	Camera *active_cam;

	bool load_ms3d(FILE *fp);
	bool load_3ds(FILE *fp);
	bool load_ply(FILE *fp);
	bool load_obj(FILE *fp);

	void calc_bounds() const;

public:
	Scene();
	virtual ~Scene();

	virtual bool load(const char *fname);

	virtual void clear();
	virtual void clear_objects();
	virtual void clear_lights();
	virtual void clear_cameras();
	virtual void clear_particles();
	virtual void clear_renderfuncs();

	virtual void set_auto_destruct(const void *item, bool auto_del);
	virtual bool get_auto_destruct(const void *item) const;

	virtual bool add_object(RObject *obj);
	virtual bool add_light(Light *lt);
	virtual bool add_camera(Camera *cam);
	virtual bool add_particles(ParticleSystem *psys);
	virtual bool add_render_func(const RenderFunc &rfunc);

	virtual RObject *get_object(const char *name) const;
	virtual RObject *get_object(int idx) const;

	virtual Light *get_light(const char *name) const;
	virtual Light *get_light(int idx) const;

	virtual Camera *get_camera(const char *name) const;
	virtual Camera *get_camera(int idx) const;

	virtual bool remove_object(const char *name);

	virtual RObject **get_objects();
	virtual RObject * const *get_objects() const;
	virtual Light **get_lights();
	virtual Light * const *get_lights() const;
	virtual Camera **get_cameras();
	virtual Camera * const *get_cameras() const;
	virtual ParticleSystem **get_particles();
	virtual ParticleSystem * const *get_particles() const;
	virtual RenderFunc *get_render_funcs();
	virtual const RenderFunc *get_render_funcs() const;

	virtual int object_count() const;
	virtual int light_count() const;
	virtual int camera_count() const;
	virtual int particle_count() const;
	virtual int render_func_count() const;

	virtual bool merge(const Scene &scn);

	virtual const AABox *get_bbox() const;
	virtual const BSphere *get_bsphere() const;

	virtual void setup_lights(unsigned int msec = 0) const;
	virtual void setup_camera(unsigned int msec = 0) const;

	virtual void render(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_SCENE_H_
