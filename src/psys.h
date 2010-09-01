#ifndef HENGE_PSYS_H_
#define HENGE_PSYS_H_

#include <list>
#include "texture.h"
#include "color.h"
#include "anim.h"
#include "vmath.h"

namespace henge {

/* fuzzy scalar values
 * random variables defined as a range of values around a central,
 * with equiprobable distrubution function
 */
class fuzzy_val {
public:
	float num, range;

	fuzzy_val(float num = 0.0, float range = 0.0);
	float operator()() const;
};

/* TODO: make a fuzzy direction with polar coordinates, so the random 
 * values lie on the surface of a sphere.
 */

/* vector of the above */
class fuzzy_vec3 {
private:
	fuzzy_val x, y, z;

public:
	fuzzy_vec3(const fuzzy_val &x = fuzzy_val(), const fuzzy_val &y = fuzzy_val(), const fuzzy_val &z = fuzzy_val());
	Vector3 operator()() const;
};


/* particle abstract base class.
 * Derived from xform_node for controller functionality
 */
class particle : public xform_node {
public:
	Vector3 velocity;
	float friction;
	float size, size_start, size_end;
	float birth_time, lifespan;

	
	particle();
	particle(const Vector3 &pos, const Vector3 &vel, float friction, float lifespan);
	virtual ~particle();

	virtual void reset();

	virtual bool alive() const;

	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const = 0;
};

/* draws the particle as a textured quad */
class billboard_particle : public particle {
public:
	texture *tex;
	color start_color, end_color;
	float rot, birth_angle;
	
	color col;
	float angle;

	virtual ~billboard_particle();
	
	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const;
};

/* TODO: draws a 3D object in the position of the particle
 * note that rotational and such controllers also apply for each
 * of the particles seperately
 */
class mesh_particle : public particle {
};


struct particle_sys_params {
	fuzzy_val psize;			// particle size
	float psize_end;			// end size (end of life)
	fuzzy_val lifespan;			// lifespan in seconds
	fuzzy_val birth_rate;		// birth rate in particles per second
	Vector3 gravity;			// gravitual force to be applied to all particles
	fuzzy_vec3 shoot_dir;		// shoot direction (initial particle velocity)
	float friction;				// friction of the environment
	fuzzy_vec3 spawn_offset;	// where to spawn in relation to position
	//Curve *spawn_offset_curve;	// a spawn curve in space, relative to position, offset still counts
	//fuzzy_val spawn_offset_curve_area;
	texture *billboard_tex;		// texture used for billboards
	color start_color;			// start color
	color end_color;			// end color
	float rot;					// particle rotation (radians / second counting from birth)
	float glob_rot;				// particle emmiter rotation, particles inherit this

	unsigned int src_blend, dest_blend;
	
	texture *halo;				// halo texture
	color halo_color;			// halo color
	fuzzy_val halo_size;		// halo size
	float halo_rot;				// halo rotation (radians / second)

	bool big_particles;			// need support for big particles (i.e. don't use point sprites)
	int max_active_particles;	// hard limit to the active particle count (-1 = no limit)

	particle_sys_params();
	bool load(const char *fname);
};

enum particle_type {PTYPE_PSYS, PTYPE_BILLBOARD, PTYPE_MESH};

/* particle system
 * The design here gets a bit confusing but for good reason
 * the particle system is also a particle because it can be emmited by
 * another particle system. This way we get a tree structure of particle
 * emmiters with the leaves being just billboards or mesh-particles.
 */
class particle_system : public particle {
protected:
	bool active, visible;
	float timeslice;

	bool ready;
	bool psprites_unsupported;
	std::list<particle*> particles;
	int num_particles;

	particle_sys_params psys_params;
	particle_type ptype;

	float fraction;
	float prev_update;
	Vector3 prev_pos;

	// current variables are calculated during each update()
	float curr_time;
	Vector3 curr_pos;
	float curr_rot, curr_halo_rot;

	particle *(*part_alloc)();

public:
	particle_system(const char *fname = 0);
	virtual ~particle_system();

	virtual void set_particle_alloc(particle *(*func)());

	virtual void reset();
	virtual void set_update_interval(float timeslice);

	virtual void set_active(bool active);
	virtual bool is_active() const;

	virtual void set_visible(bool vis);
	virtual bool is_visible() const;

	virtual void set_params(const particle_sys_params &psys_params);
	virtual particle_sys_params *get_params();
	virtual void set_particle_type(particle_type ptype);

	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const;
};

void set_psys_global_time(unsigned int msec);

/* set a limit to the maximum number of particles kept in the
 * memory pool (default: 500000)
 */
void set_psys_max_mempool(int max_free);

/* positive number sets a hard limit on the number of active
 * particles globally (all particle systems). -1 means no 
 * global limit (per-particle-system limits still apply)
 * default: -1 (no global limit).
 */
void set_psys_max_particles(int max_part);

// draws a billboarded quad
void draw_point(const Vector3 &p, const color &col, float size);

}	// namespace henge

#endif	// _PSYS_HPP_
