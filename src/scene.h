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

	bool load(const char *fname);

	void clear();
	void clear_objects();
	void clear_lights();
	void clear_cameras();
	void clear_materials();
	void clear_particles();
	void clear_renderfuncs();

	void set_auto_destruct(const void *item, bool auto_del);
	bool get_auto_destruct(const void *item) const;

	bool add_object(RObject *obj);
	bool add_light(Light *lt);
	bool add_camera(Camera *cam);
	bool add_particles(ParticleSystem *psys);
	bool add_render_func(const RenderFunc &rfunc);

	RObject *get_object(const char *name) const;
	RObject *get_object(int idx) const;

	Light *get_light(const char *name) const;
	Light *get_light(int idx) const;

	Camera *get_camera(const char *name) const;
	Camera *get_camera(int idx) const;

	bool remove_object(const char *name);

	RObject **get_objects();
	RObject * const *get_objects() const;
	Light **get_lights();
	Light * const *get_lights() const;
	Camera **get_cameras();
	Camera * const *get_cameras() const;
	ParticleSystem **get_particles();
	ParticleSystem * const *get_particles() const;
	RenderFunc *get_render_funcs();
	const RenderFunc *get_render_funcs() const;

	int object_count() const;
	int light_count() const;
	int camera_count() const;
	int particle_count() const;
	int render_func_count() const;

	bool merge(const Scene &scn);

	const AABox *get_bbox() const;
	const BSphere *get_bsphere() const;

	void setup_lights(unsigned int msec = 0) const;
	void setup_camera(unsigned int msec = 0) const;

	void render(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_SCENE_H_
