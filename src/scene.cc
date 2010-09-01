#include <float.h>
#include "scene.h"
#include "unicache.h"
#include "renderer.h"
#include "errlog.h"
#include "datapath.h"

using namespace std;
using namespace henge;

scene::scene()
{
	active_cam = 0;
}

scene::~scene()
{
	clear();
}

void scene::calc_bounds() const
{
	bsph.center = Vector3(0, 0, 0);
	bbox.min.x = bbox.min.y = bbox.min.z = FLT_MAX;
	bbox.max.x = bbox.max.y = bbox.max.z = -FLT_MAX;

	for(int i=0; i<object_count(); i++) {
		aabox *box = objects[i]->get_aabox();
		//bsphere *sph = objects[i]->get_bsphere();

		if(box->min.x < bbox.min.x) bbox.min.x = box->min.x;
		if(box->min.y < bbox.min.y) bbox.min.y = box->min.y;
		if(box->min.z < bbox.min.z) bbox.min.z = box->min.z;

		if(box->max.x > bbox.max.x) bbox.max.x = box->max.x;
		if(box->max.y > bbox.max.y) bbox.max.y = box->max.y;
		if(box->max.z > bbox.max.z) bbox.max.z = box->max.z;
	}
	bounds_valid = true;
}

bool scene::load(const char *fname)
{
	char path[PATH_MAX];
	if(!find_file(fname, path)) {
		error("scene file not found: %s\n", fname);
		return false;
	}

	FILE *fp;
	if(!(fp = fopen(path, "rb"))) {
		error("failed to open scene file: %s\n", path);
		return false;
	}

	char *last_slash = strrchr(path, '/');
	if(last_slash) {
		*last_slash = 0;
	}
	if(!strstr(get_path(), path)) {
		add_path(path);
	}

	if(load_ms3d(fp)) {
		fclose(fp);
		return true;
	}

	fseek(fp, 0, SEEK_SET);
	if(load_3ds(fp)) {
		fclose(fp);
		return true;
	}

	fseek(fp, 0, SEEK_SET);
	if(load_ply(fp)) {
		fclose(fp);
		return true;
	}

	fseek(fp, 0, SEEK_SET);
	if(load_obj(fp)) {
		fclose(fp);
		return true;
	}

	error("failed to load scene file: %s: unknown file format\n", fname);
	fclose(fp);
	return false;
}

void scene::clear()
{
	clear_objects();
	clear_lights();
	clear_cameras();
	clear_particles();
	clear_renderfuncs();
}

void scene::clear_objects()
{
	for(size_t i=0; i<objects.size(); i++) {
		if(get_auto_destruct(objects[i])) {
			delete objects[i];
		}
	}
	objects.clear();
}

void scene::clear_lights()
{
	for(size_t i=0; i<lights.size(); i++) {
		if(get_auto_destruct(lights[i])) {
			delete lights[i];
		}
	}
	lights.clear();
}

void scene::clear_cameras()
{
	for(size_t i=0; i<cameras.size(); i++) {
		if(get_auto_destruct(cameras[i])) {
			delete cameras[i];
		}
	}
	cameras.clear();
}

void scene::clear_particles()
{
	for(size_t i=0; i<particles.size(); i++) {
		if(get_auto_destruct(particles[i])) {
			delete particles[i];
		}
	}
	particles.clear();
}

void scene::clear_renderfuncs()
{
	rfuncs.clear();
}

void scene::set_auto_destruct(const void *item, bool auto_del)
{
	del_item[item] = auto_del;
}

bool scene::get_auto_destruct(const void *item) const
{
	map<const void*, bool>::const_iterator iter = del_item.find(item);
	if(iter == del_item.end()) {
		return true;
	}
	return iter->second;
}

bool scene::add_object(robject *obj)
{
	const char *name = obj->get_name() ? obj->get_name() : "<unnamed>";
	try {
		objects.push_back(obj);
		objmap[name] = obj;
		del_item[obj] = true;
	}
	catch(...) {
		return false;
	}

	bounds_valid = false;
	return true;
}

bool scene::add_light(light *lt)
{
	try {
		lights.push_back(lt);
		del_item[lt] = true;
	}
	catch(...) {
		return false;
	}
	return true;
}

bool scene::add_camera(camera *cam)
{
	try {
		cameras.push_back(cam);
		del_item[cam] = true;
	}
	catch(...) {
		return false;
	}

	if(!active_cam) {
		active_cam = cam;
	}

	return true;
}


bool scene::add_particles(particle_system *psys)
{
	try {
		particles.push_back(psys);
		del_item[psys] = true;
	}
	catch(...) {
		return false;
	}
	return true;
}

bool scene::add_render_func(const render_func &rfunc)
{
	try {
		rfuncs.push_back(rfunc);
	}
	catch(...) {
		return false;
	}
	return true;
}

robject *scene::get_object(const char *name) const
{
	map<string, robject*>::const_iterator iter = objmap.find(name);
	if(iter != objmap.end()) {
		return iter->second;
	}
	return 0;
}

robject *scene::get_object(int idx) const
{
	return objects[idx];
}

light *scene::get_light(const char *name) const
{
	for(size_t i=0; i<lights.size(); i++) {
		if(lights[i]->get_name()) {
			if(strcmp(lights[i]->get_name(), name) == 0) {
				return lights[i];
			}
		}
	}
	return 0;
}

light *scene::get_light(int idx) const
{
	return lights[idx];
}

camera *scene::get_camera(const char *name) const
{
	for(size_t i=0; i<cameras.size(); i++) {
		if(cameras[i]->get_name()) {
			if(strcmp(cameras[i]->get_name(), name) == 0) {
				return cameras[i];
			}
		}
	}
	return 0;
}

camera *scene::get_camera(int idx) const
{
	return cameras[idx];
}


bool scene::remove_object(const char *name)
{
	vector<robject*>::iterator iter = objects.begin();
	while(iter != objects.end()) {
		if(strcmp((*iter)->get_name(), name) == 0) {
			objects.erase(iter);
			objmap[name] = 0;
			return true;
		}
		iter++;
	}
	return false;
}

robject **scene::get_objects()
{
	return objects.empty() ? 0 : &objects[0];
}

robject * const * scene::get_objects() const
{
	return objects.empty() ? 0 : &objects[0];
}

light **scene::get_lights()
{
	return lights.empty() ? 0 : &lights[0];
}

light * const * scene::get_lights() const
{
	return lights.empty() ? 0 : &lights[0];
}

camera **scene::get_cameras()
{
	return cameras.empty() ? 0 : &cameras[0];
}

camera * const * scene::get_cameras() const
{
	return cameras.empty() ? 0 : &cameras[0];
}


particle_system **scene::get_particles()
{
	return particles.empty() ? 0 : &particles[0];
}

particle_system * const * scene::get_particles() const
{
	return particles.empty() ? 0 : &particles[0];
}

render_func *scene::get_render_funcs()
{
	return rfuncs.empty() ? 0 : &rfuncs[0];
}

const render_func *scene::get_render_funcs() const
{
	return rfuncs.empty() ? 0 : &rfuncs[0];
}

int scene::object_count() const
{
	return (int)objects.size();
}

int scene::light_count() const
{
	return (int)lights.size();
}

int scene::camera_count() const
{
	return (int)cameras.size();
}


int scene::particle_count() const
{
	return (int)particles.size();
}

int scene::render_func_count() const
{
	return (int)rfuncs.size();
}

bool scene::merge(const scene &scn)
{
	robject * const * obj = scn.get_objects();
	int num_obj = scn.object_count();

	for(int i=0; i<num_obj; i++) {
		add_object(obj[i]->clone());
	}

	light * const * lights = scn.get_lights();
	int num_lights = scn.light_count();

	for(int i=0; i<num_lights; i++) {
		add_light(lights[i]->clone());
	}

	camera * const * cam = scn.get_cameras();
	int num_cam = scn.camera_count();

	for(int i=0; i<num_cam; i++) {
		add_camera(cam[i]->clone());
	}

	particle_system * const * psys = scn.get_particles();
	int num_psys = scn.particle_count();

	for(int i=0; i<num_psys; i++) {
		particle_system *newpsys = new particle_system(*(psys[i]));
		add_particles(newpsys);
	}

	return true;
}

const aabox *scene::get_bbox() const
{
	if(!bounds_valid) {
		calc_bounds();
	}
	return &bbox;
}

const bsphere *scene::get_bsphere() const
{
	if(!bounds_valid) {
		calc_bounds();
	}
	return &bsph;
}

void scene::setup_lights(unsigned int msec) const
{
	if(lights.empty()) {
		return;
	}

	int lidx = 0;
	int num_lights = 0;
	for(int i=0; i<caps.max_lights; i++) {
		if(i < (int)lights.size()) {
			if(lights[i]->bind(lidx, msec)) {
				lidx++;
				num_lights++;
			}
		} else {
			glDisable(GL_LIGHT0 + lidx++);
		}
	}

	cache_uniform("uc_num_lights", num_lights);
}

void scene::setup_camera(unsigned int msec) const
{
	if(active_cam) {
		active_cam->bind(msec);
	}
}

void scene::render(unsigned int msec) const
{
	get_renderer()->render(this, msec);
}

/*
void scene::render(unsigned int msec) const
{
	current_time = msec;

	if(rend_mask & REND_CAM) {
		setup_camera(msec);
	}
	if(rend_mask & REND_LIGHTS) {
		setup_lights(msec);
	}

	// TODO handle shadows and reflections


	list<robject*> transp_obj;

	if(rend_mask & REND_OBJ) {
		// opaque objects pass, push transparent ones on another list for
		// sorting back->front and rendering separately.
		for(size_t i=0; i<objects.size(); i++) {
			const material *mat = objects[i]->get_material_ptr();
			if(mat && mat->is_transparent()) {
				transp_obj.push_back(objects[i]);
			} else {
				objects[i]->render(msec);
			}
		}
	}

	if(rend_mask & REND_RFUNCS) {
		// also call the render funcs before the transparent objects (?)
		// XXX this is wrong by definition, but can't do any better
		for(size_t i=0; i<rfuncs.size(); i++) {
			rfuncs[i](msec);
		}
	}

	if(rend_mask & REND_OBJ) {
		// transparent objects pass
		transp_obj.sort(obj_cmp);

		list<robject*>::const_iterator iter = transp_obj.begin();
		while(iter != transp_obj.end()) {
			(*iter++)->render(msec);
		}
	}

	if(rend_mask & REND_PSYS) {
		// update and render the particle systems
		set_psys_global_time(msec);
		for(size_t i=0; i<particles.size(); i++) {
			particles[i]->update();
			particles[i]->draw();
		}
	}
}
*/
