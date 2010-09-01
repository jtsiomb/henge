#include <stdio.h>
#include "scene.h"
#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>
#include <lib3ds/io.h>

using namespace henge;

static int load_objects(scene *scn, Lib3dsFile *file);
static int load_lights(scene *scn, Lib3dsFile *file);
static int load_cameras(scene *scn, Lib3dsFile *file);

static void load_material(material *mat, Lib3dsMaterial *mat3ds);
static bool load_mesh(trimesh *mesh, Lib3dsMesh *mesh3ds);

static Vector3 mkvector(float *vec);
static color mkcolor(float *vec);

static Lib3dsBool ioerror(void *fp);
static long ioseek(void *fp, long offset, Lib3dsIoSeek whence);
static long iotell(void *fp);
static size_t ioread(void *fp, void *buffer, size_t size);
static size_t iowrite(void *fp, const void *buffer, size_t size);


bool scene::load_3ds(FILE *fp)
{
	Lib3dsFile *file;
	Lib3dsIo *io;

	if(!(file = lib3ds_file_new())) {
		return false;
	}

	if(!(io = lib3ds_io_new(fp, ioerror, ioseek, iotell, ioread, iowrite))) {
		lib3ds_file_free(file);
		return false;
	}

	if(!lib3ds_file_read(file, io)) {
		lib3ds_file_free(file);
		lib3ds_io_free(io);
		return false;
	}
	lib3ds_file_eval(file, 0);

	load_objects(this, file);
	load_lights(this, file);
	load_cameras(this, file);

	lib3ds_file_free(file);
	lib3ds_io_free(io);

	return true;
}

static int load_objects(scene *scn, Lib3dsFile *file)
{
	Lib3dsNode *node;
	Lib3dsMesh *m = file->meshes;

	int num_obj = 0;
	while(m) {
		if(!m->faces || !m->points) {	// skip non-meshes
			m = m->next;
			continue;
		}

		if(!(node = lib3ds_file_node_by_name(file, m->name, LIB3DS_OBJECT_NODE))) {
			warning("mesh %s does not have a node!\n", m->name);
		}

		robject *obj = new robject;
		obj->set_name(m->name);

		// load the material
		if(m->faceL[0].material && *m->faceL[0].material) {
			Lib3dsMaterial *mat3ds = lib3ds_file_material_by_name(file, m->faceL[0].material);
			load_material(obj->get_material_ptr(), mat3ds);
		}
		
		// load the geometry
		load_mesh(obj->get_mesh(), m);

		printf("adding object: %s\n", m->name);
		scn->add_object(obj);
		num_obj++;
		
		m = m->next;	// next mesh...
	}
	return num_obj;
}

static void load_material(material *mat, Lib3dsMaterial *mat3ds)
{
	mat->set_color(mkcolor(mat3ds->ambient), MATTR_AMBIENT);
	mat->set_color(mkcolor(mat3ds->diffuse), MATTR_DIFFUSE);

	float shin_str = mat3ds->shin_strength;
	mat->set_color(mkcolor(mat3ds->specular) * shin_str, MATTR_SPECULAR);

	mat->set(mat3ds->shininess * 100.0, MATTR_SHININESS);
	mat->set(1.0 - mat3ds->transparency, MATTR_ALPHA);

	// texture maps
	int map_idx = 0;
	if(mat3ds->texture1_map.name) {
		mat->set_texture(get_texture(mat3ds->texture1_map.name), map_idx++);

		Vector3 offs = Vector3(mat3ds->texture1_map.offset[0], mat3ds->texture1_map.offset[1], 0.0);
		Vector3 scale = Vector3(mat3ds->texture1_map.scale[0], mat3ds->texture1_map.scale[1], 1.0);
		float rot = mat3ds->texture1_map.rotation;
		
		xform_node *xform = mat->texture_xform();
		xform->set_position(offs);
		xform->set_rotation(Vector3(0, 0, rot));
		xform->set_scaling(scale);
	}
	if(mat3ds->specular_map.name) {
		mat->set_texture(get_texture(mat3ds->specular_map.name), map_idx++);
	}
	if(mat3ds->bump_map.name) {
		mat->set_texture(get_texture(mat3ds->bump_map.name), map_idx++);
	}
	if(mat3ds->reflection_map.name) {
		mat->set_texture(get_texture(mat3ds->reflection_map.name), map_idx++);
	}
}

static bool load_mesh(trimesh *mesh, Lib3dsMesh *mesh3ds)
{
	Vector3 *verts, *norms;
	Vector2 *tc = 0;
	// for now let's do it unindexed
	int num_verts = mesh3ds->faces * 3;

	mesh->set_data(EL_VERTEX, (Vector3*)0, num_verts);
	verts = mesh->get_data_vec3(EL_VERTEX);

	mesh->set_data(EL_NORMAL, (Vector3*)0, num_verts);
	norms = mesh->get_data_vec3(EL_NORMAL);

	if(mesh3ds->texels) {
		mesh->set_data(EL_TEXCOORD, (Vector2*)0, num_verts);
		tc = mesh->get_data_vec2(EL_TEXCOORD);
	}

	Lib3dsVector *n3ds = (Lib3dsVector*)malloc(3 * sizeof *n3ds * mesh3ds->faces);
	lib3ds_mesh_calculate_normals(mesh3ds, n3ds);

	for(int i=0; i<(int)mesh3ds->faces; i++) {
		for(int j=0; j<3; j++) {
			int idx = mesh3ds->faceL[i].points[j];
			int nidx = i * 3 + j;

			*verts++ = mkvector(mesh3ds->pointL[idx].pos);
			*norms++ = mkvector((float*)(n3ds + nidx));

			if(tc) {
				*tc++ = Vector2(mesh3ds->texelL[idx][0], mesh3ds->texelL[idx][1]);
			}
		}
	}

	free(n3ds);
	return true;
}

static int load_lights(scene *scn, Lib3dsFile *file)
{
	Lib3dsLight *lt = file->lights;

	int num_lt = 0;
	while(lt) {
		light *light;
		if(lt->spot_light) {
			spotlight *spot = new spotlight;
			light = spot;

			spot->set_direction(mkvector(lt->spot));
			spot->set_cone(DEG_TO_RAD(lt->inner_range), DEG_TO_RAD(lt->outer_range));
		} else {
			light = new henge::light;
		}

		color lcol = mkcolor(lt->color) * lt->multiplier;

		light->set_position(lcol);
		light->set_specular(lcol);

		if(lt->attenuation > XSMALL_NUMBER) {
			light->set_attenuation(0.0, lt->attenuation, 0.0);
		}
		if(lt->off) light->disable();

		scn->add_light(light);
		num_lt++;

		lt = lt->next;
	}

	return num_lt;
}

static int load_cameras(scene *scn, Lib3dsFile *file)
{
	Lib3dsCamera *c = file->cameras;

	int num_cam = 0;
	while(c) {
		target_camera *cam = new target_camera;

		cam->set_position(mkvector(c->position));
		cam->set_target(mkvector(c->target));
		cam->set_roll(c->roll);

		scn->add_camera(cam);
		num_cam++;

		c = c->next;
	}

	return num_cam;
}

static Vector3 mkvector(float *vec)
{
	return Vector3(vec[0], vec[2], vec[1]);
}

static color mkcolor(float *vec)
{
	return color(vec[0], vec[1], vec[2]);
}


static Lib3dsBool ioerror(void *fp)
{
	return ferror((FILE*)fp);
}

static long ioseek(void *fp, long offset, Lib3dsIoSeek whence)
{
	int w;
	switch(whence) {
	case LIB3DS_SEEK_SET:
		w = SEEK_SET;
		break;

	case LIB3DS_SEEK_CUR:
		w = SEEK_CUR;
		break;

	case LIB3DS_SEEK_END:
		w = SEEK_END;
		break;

	default:
		return 0;
	}
	return fseek((FILE*)fp, offset, w);
}

static long iotell(void *fp)
{
	return ftell((FILE*)fp);
}

static size_t ioread(void *fp, void *buffer, size_t size)
{
	return fread(buffer, 1, size, (FILE*)fp);
}

static size_t iowrite(void *fp, const void *buffer, size_t size)
{
	return fwrite(buffer, 1, size, (FILE*)fp);
}

