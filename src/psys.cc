#include <vector>
#include <math.h>
#include "psys.h"
#include "cfgfile.h"
#include "errlog.h"
#include "material.h"


using namespace std;
using namespace henge;

#define MAX_FREE_LIST_SIZE	500000

/* change this to a positive number, for a hard cap on the number of particles */
#define MAX_PARTICLES		-1

// see: set_psys_max_mempool()
static int max_free_list_size = 500000;

// see: set_psys_max_particles()
static int max_active_particles = -1;

// prototypes of the particle allocator (memory pool)
static Particle *new_particle();
static void delete_particle(Particle *p);

static float global_time;

/* just a trial and error constant to match point-sprite size with
 * billboard size
 */
#define PSPRITE_BILLBOARD_RATIO		100

// particle rendering state
static bool use_psprites = true;
static bool volatile_particles = false;

void henge::set_psys_global_time(unsigned int msec)
{
	global_time = (float)msec / 1000.0;
}


FuzzyVal::FuzzyVal(float num, float range)
{
	this->num = num;
	this->range = range;
}

float FuzzyVal::operator()() const
{
	return range == 0.0 ? num : frand(range) + num - range / 2.0;
}


FuzzyVec3::FuzzyVec3(const FuzzyVal &x, const FuzzyVal &y, const FuzzyVal &z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3 FuzzyVec3::operator()() const
{
	return Vector3(x(), y(), z());
}


Particle::Particle()
{
	reset();
}

Particle::Particle(const Vector3 &pos, const Vector3 &vel, float friction, float lifespan)
{
	set_position(pos);
	velocity = vel;
	this->friction = friction;
	this->lifespan = lifespan;
	birth_time = global_time;
}

Particle::~Particle() {}

void Particle::reset()
{
	friction = 1.0;
	lifespan = 0;
	birth_time = 0;
}

bool Particle::alive() const
{
	return global_time - birth_time < lifespan;
}

void Particle::update(const Vector3 &ext_force)
{
	float time = global_time - birth_time;
	if(time > lifespan) return;

	velocity = (velocity + ext_force) * friction;
	translate(velocity);	// update position
}

BillboardParticle::~BillboardParticle() {}

void BillboardParticle::update(const Vector3 &ext_force)
{
	Particle::update(ext_force);

	float time = global_time - birth_time;
	if(time > lifespan) return;
	float t = time / lifespan;

	col = lerp(start_color, end_color, t);

	size = size_start + (size_end - size_start) * t;

	angle = rot * time + birth_angle;
}

/* NOTE:
 * if we use point sprites, and the particles are not rotating, then the
 * calling function has taken care to call glBegin() before calling this.
 */
void BillboardParticle::draw() const
{
	Matrix4x4 tex_rot;
	if(volatile_particles) {
		tex_rot.translate(Vector3(0.5, 0.5, 0.0));
		tex_rot.rotate(Vector3(0.0, 0.0, angle));
		tex_rot.translate(Vector3(-0.5, -0.5, 0.0));

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		load_matrix(tex_rot);
	}

	Vector3 pos = get_position();

	if(use_psprites) {
		if(volatile_particles) {
			glPointSize(size);

			glBegin(GL_POINTS);
			glColor4f(col.x, col.y, col.z, col.w);
			glVertex3f(pos.x, pos.y, pos.z);
			glEnd();
		} else {
			glColor4f(col.x, col.y, col.z, col.w);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	} else {	// don't use point sprites
		draw_point(pos, col, size / PSPRITE_BILLBOARD_RATIO);
	}

	if(volatile_particles) {
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
	}
}


ParticleSystem::ParticleSystem(const char *fname)
{
	timeslice = 1.0f / 50.0f;		// that's the default timeslice
	num_particles = 0;

	active = true;
	visible = true;

	psprites_unsupported = !caps.psprites;

	prev_update = -1.0;
	fraction = 0.0;
	ptype = PTYPE_BILLBOARD;

	ready = true;

	if(fname) {
		if(!psys_params.load(fname)) {
			error("error loading particle file: %s\n", fname);
			ready = false;
		}
	}

	part_alloc = new_particle;
}

ParticleSystem::~ParticleSystem()
{
	reset();
}

void ParticleSystem::set_particle_alloc(Particle *(*func)())
{
	part_alloc = func;
}

void ParticleSystem::reset()
{
	prev_update = -1.0;
	std::list<Particle*>::iterator iter = particles.begin();
	while(iter != particles.end()) {
		delete *iter++;
	}
	particles.clear();
}

void ParticleSystem::set_update_interval(float timeslice)
{
	this->timeslice = timeslice;
}

void ParticleSystem::set_active(bool active)
{
	this->active = active;
}

bool ParticleSystem::is_active() const
{
	return active;
}


void ParticleSystem::set_visible(bool visible)
{
	this->visible = visible;
}

bool ParticleSystem::is_visible() const
{
	return visible;
}

void ParticleSystem::set_params(const ParticleSysParams &psys_params)
{
	this->psys_params = psys_params;
}

ParticleSysParams *ParticleSystem::get_params()
{
	return &psys_params;
}

void ParticleSystem::set_particle_type(ParticleType ptype)
{
	this->ptype = ptype;
}

void ParticleSystem::update(const Vector3 &ext_force)
{
	if(!ready) {// || (!active && num_particles == 0)) {
		return;
	}

	curr_time = global_time;
	int updates_missed = (int)round((global_time - prev_update) / timeslice);

	if(!updates_missed) return;	// less than a timeslice has elapsed, nothing to do

	Vector3 pos;
	curr_pos = pos.transformed(get_xform_matrix((unsigned int)(global_time * 1000.0)));
	//curr_pos = get_position((unsigned int)(global_time * 1000.0));
	curr_halo_rot = psys_params.halo_rot * global_time;

	curr_rot = fmod(psys_params.glob_rot * global_time, 2.0f * (float)M_PI);

	// spawn new particles
	if(active) {
		float spawn = psys_params.birth_rate() * (global_time - prev_update);
		int spawn_count = (int)round(spawn);

		// handle sub-timeslice spawning rates
		fraction += spawn - round(spawn);
		if(fraction > 1.0) {
			fraction -= 1.0;
			spawn_count++;
		} else if(fraction < -1.0) {
			fraction += 1.0;
			spawn_count--;
		}

		Vector3 dp, pos;
		if(prev_update < 0.0) {
			prev_pos = curr_pos;
			prev_update = curr_time;
			return;
		} else {
			dp = (curr_pos - prev_pos) / (float)spawn_count;
			pos = prev_pos;
		}

		float dt = (global_time - prev_update) / (float)spawn_count;
		float t = prev_update;

		for(int i=0; i<spawn_count; i++) {
			if(psys_params.max_active_particles >= 0 &&
					num_particles >= psys_params.max_active_particles) {
				break;
			}

			Particle *p;
			switch(ptype) {
			case PTYPE_BILLBOARD:
				if((p = part_alloc())) {
					curr_rot = fmod(psys_params.glob_rot * t, 2.0f * (float)M_PI);

					BillboardParticle *bbp = (BillboardParticle*)p;
					bbp->tex = psys_params.billboard_tex;
					bbp->start_color = psys_params.start_color;
					bbp->end_color = psys_params.end_color;
					bbp->rot = psys_params.rot;
					bbp->birth_angle = curr_rot;
				}
				break;

			default:
				error("Only billboarded particles implemented currently");
				return;
			}

			if(!p) continue;

			num_particles++;

			Vector3 offset = psys_params.spawn_offset();
			/*
			if(psys_params.spawn_offset_curve) {
				float t = psys_params.spawn_offset_curve_area();
				offset += (*psys_params.spawn_offset_curve)(t);
			}
			*/
			// XXX: correct this rotation to span the whole interval
			Quaternion rot = get_rotation();
			p->set_position(pos + offset.transformed(rot));
			p->set_rotation(rot);
			p->set_scaling(get_scaling());

			p->size_start = psys_params.psize();
			if(psys_params.psize_end < 0.0) {
				p->size_end = p->size_start;
			} else {
				p->size_end = psys_params.psize_end;
			}

			// XXX: correct this next rotation to span the interval
			p->velocity = psys_params.shoot_dir().transformed(rot);
			p->friction = psys_params.friction;
			p->birth_time = t;
			p->lifespan = psys_params.lifespan();

			particles.push_back(p);

			pos += dp;
			t += dt;
		}
	}


	// update particles
	std::list<Particle*>::iterator iter = particles.begin();
	while(iter != particles.end()) {
		Particle *p = *iter;
		int i = 0;
		while(p->alive() && i++ < updates_missed) {
			p->update(psys_params.gravity);
		}

		if(p->alive()) {
			iter++;
		} else {
			delete_particle(*iter);
			iter = particles.erase(iter);
			num_particles--;
		}
	}

	prev_update = global_time;
	prev_pos = curr_pos;
}

void ParticleSystem::draw() const
{
	if(!ready || !visible) return;

	// use point sprites if the system supports them AND we don't need big particles
	use_psprites = !psys_params.big_particles && !psprites_unsupported;

	// particles are volatile if they rotate OR they fluctuate in size
	volatile_particles = psys_params.rot > SMALL_NUMBER || psys_params.psize.range > SMALL_NUMBER;

	std::list<Particle*>::const_iterator iter = particles.begin();
	if(iter != particles.end()) {

		if(ptype == PTYPE_BILLBOARD) {
			// ------ setup render state ------
			glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT |
					GL_DEPTH_BUFFER_BIT | GL_POINT_BIT);

			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glEnable(GL_BLEND);
			glBlendFunc(psys_params.src_blend, psys_params.dest_blend);

			if(psys_params.billboard_tex) {
				if(get_mat_bind_mask() & MAT_BIND_TEXTURE) {
					psys_params.billboard_tex->bind();
				}
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				if(use_psprites) {
					glEnable(GL_POINT_SPRITE_ARB);
					glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, 1);
				}

				if(!volatile_particles) {
					Matrix4x4 prot;
					prot.translate(Vector3(0.5, 0.5, 0.0));
					prot.rotate(Vector3(0.0, 0.0, curr_rot));
					prot.translate(Vector3(-0.5, -0.5, 0.0));

					glMatrixMode(GL_TEXTURE);
					glPushMatrix();
					load_matrix(prot);
				}
			}

			if(use_psprites && !volatile_particles) {
				glPointSize((*iter)->size);
				glBegin(GL_POINTS);
			}
		}

		// ------ render particles ------
		while(iter != particles.end()) {
			(*iter++)->draw();
		}

		if(ptype == PTYPE_BILLBOARD) {
			// ------ restore render states -------
			if(use_psprites) {
				if(!volatile_particles) glEnd();
			}

			if(psys_params.billboard_tex && !volatile_particles) {
				glMatrixMode(GL_TEXTURE);
				glPopMatrix();
			}
			glPopAttrib();
		}
	}

	// ------ render a halo around the emitter if we need to ------
	if(psys_params.halo) {
		// construct texture matrix for halo rotation
		Matrix4x4 mat;
		mat.translate(Vector3(0.5, 0.5, 0.0));
		mat.rotate(Vector3(0, 0, curr_halo_rot));
		mat.translate(Vector3(-0.5, -0.5, 0.0));

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		load_matrix(mat);

		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		psys_params.halo->bind();

		glDepthMask(0);

		float sz = psys_params.halo_size() / PSPRITE_BILLBOARD_RATIO;
		draw_point(curr_pos, psys_params.halo_color, sz);

		glPopAttrib();

		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
	}
}

void henge::set_psys_max_mempool(int max_free)
{
	max_free_list_size = max_free;
}

void henge::set_psys_max_particles(int max_part)
{
	max_active_particles = max_part;
}

void henge::draw_point(const Vector3 &p, const Color &col, float size)
{
	float m[16];

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(p.x, p.y, p.z);

	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	// make the upper 3x3 part of the matrix identity
	for(int i=0; i<10; i++) {
		m[i] = i % 5 ? 0.0f : 1.0f;
	}
	glLoadMatrixf(m);
	//glScalef(size, size, size);

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	glColor4f(col.x, col.y, col.z, col.w);
	glTexCoord2f(0, 0);
	glVertex2f(-size, -size);
	glTexCoord2f(1, 0);
	glVertex2f(size, -size);
	glTexCoord2f(1, 1);
	glVertex2f(size, size);
	glTexCoord2f(0, 1);
	glVertex2f(-size, size);
	glEnd();

	glPopAttrib();
	glPopMatrix();
}



static unsigned int get_blend_factor(const char *str);


ParticleSysParams::ParticleSysParams()
{
	psize_end = -1.0;
	friction = 0.95f;
	billboard_tex = 0;
	halo = 0;
	rot = 0.0;
	glob_rot = 0.0;
	halo_rot = 0.0;
	big_particles = false;
	max_active_particles = -1;
	//spawn_offset_curve = 0;
	//spawn_offset_curve_area = FuzzyVal(0.5, 1.0);

	start_color = end_color = halo_color = Color(1, 1, 1, 1);

	src_blend = GL_SRC_ALPHA;
	dest_blend = GL_ONE;
}


bool ParticleSysParams::load(const char *fname)
{
	float val;
	Vector4 vec;
	string str;

	Vector3 tmp_shoot, tmp_shoot_range;
	Vector3 tmp_spawn_off, tmp_spawn_off_range;

	ConfigFile cfg;
	if(!(cfg.read(fname))) {
		return false;
	}

	if(cfg.getopt("psize", &val)) {
		psize.num = val;
	}
	if(cfg.getopt("psize-r", &val)) {
		psize.range = val;
	}
	if(cfg.getopt("psize_end", &val)) {
		psize_end = val;
	}
	if(cfg.getopt("life", &val)) {
		lifespan.num = val;
	}
	if(cfg.getopt("life-r", &val)) {
		lifespan.range = val;
	}
	if(cfg.getopt("birth-rate", &val)) {
		birth_rate.num = val;
	}
	if(cfg.getopt("birt-rate-r", &val)) {
		birth_rate.range = val;
	}
	if(cfg.getopt("grav", &vec)) {
		gravity = vec;
	}
	if(cfg.getopt("shoot", &vec)) {
		tmp_shoot = vec;
	}
	if(cfg.getopt("shoot-r", &vec)) {
		tmp_shoot_range = vec;
	}
	if(cfg.getopt("friction", &val)) {
		friction = val;
	}
	if(cfg.getopt("spawn_off", &vec)) {
		tmp_spawn_off = vec;
	}
	if(cfg.getopt("spawn_off-r", &vec)) {
		tmp_spawn_off_range = vec;
	}
	if(cfg.getopt("tex", &str)) {
		billboard_tex = get_texture(str.c_str());
	}
	if(cfg.getopt("color", &vec)) {
		start_color = end_color = vec;
	}
	if(cfg.getopt("color_start", &vec)) {
		start_color = vec;
	}
	if(cfg.getopt("color_end", &vec)) {
		end_color = vec;
	}
	if(cfg.getopt("rot", &val)) {
		rot = val;
	}
	if(cfg.getopt("glob_rot", &val)) {
		glob_rot = val;
	}
	if(cfg.getopt("halo", &str)) {
		halo = get_texture(str.c_str());
	}
	if(cfg.getopt("halo_color", &vec)) {
		halo_color = vec;
	}
	if(cfg.getopt("halo_size", &val)) {
		halo_size.num = val;
	}
	if(cfg.getopt("halo_size-r", &val)) {
		halo_size.range = val;
	}
	if(cfg.getopt("halo_rot", &val)) {
		halo_rot = val;
	}
	if(cfg.getopt("max_particles", &val)) {
		max_active_particles = (int)val;
	}
	if(cfg.getopt("big_particles", &str)) {
		if(str == "true") {
			big_particles = true;
		}
	}
	if(cfg.getopt("blend_src", &str)) {
		unsigned int factor = get_blend_factor(str.c_str());
		if(factor != 0xbadbad) {
			src_blend = factor;
		}
	}
	if(cfg.getopt("blend_dest", &str)) {
		unsigned int factor = get_blend_factor(str.c_str());
		if(factor != 0xbadbad) {
			dest_blend = factor;
		}
	}
	if(cfg.getopt("spawn_offset_curve", &str)) {
		// TODO load curve...
	}


	shoot_dir = FuzzyVec3(FuzzyVal(tmp_shoot.x, tmp_shoot_range.x),
			FuzzyVal(tmp_shoot.y, tmp_shoot_range.y),
			FuzzyVal(tmp_shoot.z, tmp_shoot_range.z));
	spawn_offset = FuzzyVec3(FuzzyVal(tmp_spawn_off.x, tmp_spawn_off_range.x),
			FuzzyVal(tmp_spawn_off.y, tmp_spawn_off_range.y),
			FuzzyVal(tmp_spawn_off.z, tmp_spawn_off_range.z));

	return true;
}

static struct {
	const char *name;
	unsigned int factor;
} blend_mode[] = {
	{"0", GL_ZERO},
	{"1", GL_ONE},
	{"srcc", GL_SRC_COLOR},
	{"srca", GL_SRC_ALPHA},
	{"1-srcc", GL_ONE_MINUS_SRC_COLOR},
	{"1-srca", GL_ONE_MINUS_SRC_ALPHA},
	{"1-dstc", GL_ONE_MINUS_DST_COLOR},
	{"1-dsta", GL_ONE_MINUS_DST_ALPHA},
	{0, 0}
};

static unsigned int get_blend_factor(const char *str)
{
	for(int i=0; blend_mode[i].name; i++) {
		if(strcmp(blend_mode[i].name, str) == 0) {
			return blend_mode[i].factor;
		}
	}
	return 0xbadbad;
}

// ---- particle memory allocator ----
static std::list<Particle*> free_list;
static size_t free_list_size;
static int active_particles;

static Particle *new_particle()
{
	if(max_active_particles >= 0 && active_particles >= max_active_particles) {
		return 0;
	}

	Particle *p;
	if(free_list.empty()) {
		p = new BillboardParticle;
	} else {
		p = *free_list.begin();
		free_list.erase(free_list.begin());
		p->reset();	// re-initialize everything
		free_list_size--;
	}

	active_particles++;
	return p;
}

static void delete_particle(Particle *p)
{
	if(free_list_size < MAX_FREE_LIST_SIZE) {
		free_list.push_front(p);
		free_list_size++;
	} else {
		delete p;
	}
}
