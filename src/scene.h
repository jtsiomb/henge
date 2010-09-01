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

class scene {
protected:
	std::map<std::string, robject*> objmap;

	std::vector<robject*> objects;
	std::vector<light*> lights;
	std::vector<camera*> cameras;
	std::vector<particle_system*> particles;
	std::vector<render_func> rfuncs;

	mutable aabox bbox;
	mutable bsphere bsph;
	mutable bool bounds_valid;

	// maps each item (object/light/etc) to a flag controlling
	// automatic deletion of the item when clean is called.
	std::map<const void*, bool> del_item;

	camera *active_cam;

	bool load_ms3d(FILE *fp);
	bool load_3ds(FILE *fp);
	bool load_ply(FILE *fp);
	bool load_obj(FILE *fp);

	void calc_bounds() const;

public:
	scene();
	virtual ~scene();

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

	bool add_object(robject *obj);
	bool add_light(light *lt);
	bool add_camera(camera *cam);
	bool add_particles(particle_system *psys);
	bool add_render_func(const render_func &rfunc);

	robject *get_object(const char *name) const;
	robject *get_object(int idx) const;

	light *get_light(const char *name) const;
	light *get_light(int idx) const;

	camera *get_camera(const char *name) const;
	camera *get_camera(int idx) const;

	bool remove_object(const char *name);

	robject **get_objects();
	robject * const *get_objects() const;
	light **get_lights();
	light * const *get_lights() const;
	camera **get_cameras();
	camera * const *get_cameras() const;
	particle_system **get_particles();
	particle_system * const *get_particles() const;
	render_func *get_render_funcs();
	const render_func *get_render_funcs() const;

	int object_count() const;
	int light_count() const;
	int camera_count() const;
	int particle_count() const;
	int render_func_count() const;

	bool merge(const scene &scn);

	const aabox *get_bbox() const;
	const bsphere *get_bsphere() const;

	void setup_lights(unsigned int msec = 0) const;
	void setup_camera(unsigned int msec = 0) const;

	void render(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_SCENE_H_
