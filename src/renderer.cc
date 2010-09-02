#include <list>
#include "renderer.h"

using namespace std;
using namespace henge;

namespace henge {
	unsigned int current_time;
}

static bool obj_cmp(const RObject *r1, const RObject *r2);

static StdRenderer def_rend;
static Renderer *act_rend = &def_rend;


void henge::set_renderer(Renderer *rend)
{
	act_rend = rend;
}

Renderer *henge::get_renderer()
{
	return act_rend;
}


Renderer::Renderer()
{
	rend_mask = REND_ALL;
}

Renderer::~Renderer() {}

void Renderer::set_render_mask(unsigned int rmask)
{
	rend_mask = rmask;
}

unsigned int Renderer::get_render_mask() const
{
	return rend_mask;
}

//StdRenderer::~StdRenderer()
//{
//}

void StdRenderer::render(const Scene *scn, unsigned int msec) const
{
	current_time = msec;

	if(rend_mask & REND_CAM) {
		scn->setup_camera(msec);
	}
	if(rend_mask & REND_LIGHTS) {
		scn->setup_lights(msec);	// TODO get rid of this
	}

	list<RObject*> transp_obj;

	if(rend_mask & REND_OBJ) {
		// opaque objects pass, push transparent ones on another list for
		// sorting back->front and rendering separately.
		RObject * const *obj = scn->get_objects();
		int num_obj = scn->object_count();

		for(int i=0; i<num_obj; i++) {
			const Material *mat = obj[i]->get_material_ptr();
			if(mat->is_transparent()) {
				if(rend_mask & REND_TRANSPARENT) {
					transp_obj.push_back(obj[i]);
				}
			} else {
				obj[i]->render(msec);
			}
		}
	}

	if(rend_mask & REND_RFUNCS) {
		// also call the render funcs before the transparent objects
		// XXX this is wrong by definition, but can't do any better.
		const RenderFunc *rfunc = scn->get_render_funcs();
		int num_rfunc = scn->render_func_count();

		for(int i=0; i<num_rfunc; i++) {
			rfunc[i](msec);
		}
	}


	if(rend_mask & REND_OBJ & REND_TRANSPARENT) {
		// transparent objects pass
		transp_obj.sort(obj_cmp);

		list<RObject*>::const_iterator iter = transp_obj.begin();
		while(iter != transp_obj.end()) {
			(*iter++)->render(msec);
		}
	}

	if(rend_mask & REND_PSYS) {
		// update and render the particle systems
		ParticleSystem * const *psys = scn->get_particles();
		int num_psys = scn->particle_count();

		set_psys_global_time(msec);
		for(int i=0; i<num_psys; i++) {
			psys[i]->update();
			psys[i]->draw();
		}
	}
}


static bool obj_cmp(const RObject *r1, const RObject *r2)
{
	return *r1 < *r2;
}

