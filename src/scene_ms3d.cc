#include <map>
#include <string>
#include "scene.h"
#include "errlog.h"

using namespace std;
using namespace henge;

#ifdef __GNUC__
#define PACKED	__attribute__((packed))
#else
#define PACKED
#endif

#pragma pack (push,1)
struct ms3d_header {
	char magic[10];
	int fmt_ver;
} PACKED;

struct ms3d_vertex {
	char flags;
	float pos[3];
	char bone_id;		// -1 means no bone
	char ref_count;
} PACKED;

struct ms3d_triangle {
	uint16_t flags;
	uint16_t v[3];
	float vnorm[3][3];
	float s[3];
	float t[3];
	unsigned char smoothing_group;
	unsigned char group_idx;
} PACKED;

struct ms3d_material {
	char name[32];
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;
	float transparency;
	char mode;
	char texture[128];
	char alphamap[128];
} PACKED;
#pragma pack (pop)

class ms3d_group {
public:
	char flags;
	char name[32];
	uint16_t tri_count;
	uint16_t *tri_idx;
	char matref;

	ms3d_group();
	~ms3d_group();
};

class ms3d_file {
public:
	ms3d_header hdr;
	ms3d_vertex *vert;
	ms3d_triangle *tri;
	ms3d_group *grp;
	ms3d_material *mat;
	int num_vert, num_tri, num_grp, num_mat;

	ms3d_file();
	~ms3d_file();
};

#define UNEXP_EOF	"load_ms3d: premature end of file encountered\n"

static bool read_header(FILE *fp, ms3d_header *hdr);
static ms3d_vertex *read_vertices(FILE *fp, int *count);
static ms3d_triangle *read_triangles(FILE *fp, int *count);
static ms3d_group *read_groups(FILE *fp, int *count);
static ms3d_material *read_materials(FILE *fp, int *count);
static robject *cons_object(const ms3d_file *ms3d, const ms3d_group *grp);

static map<string, material> matlib;

bool scene::load_ms3d(FILE *fp)
{
	ms3d_file ms3d;

	if(!read_header(fp, &ms3d.hdr)) {
		return false;
	}

	if(!(ms3d.vert = read_vertices(fp, &ms3d.num_vert))) {
		return false;
	}

	if(!(ms3d.tri = read_triangles(fp, &ms3d.num_tri))) {
		return false;
	}

	if(!(ms3d.grp = read_groups(fp, &ms3d.num_grp))) {
		return false;
	}

	if(!(ms3d.mat = read_materials(fp, &ms3d.num_mat))) {
		return false;
	}

	// add all materials to the scene material lib
	for(int i=0; i<ms3d.num_mat; i++) {
		ms3d_material *m = ms3d.mat + i;

		color col_amb(m->ambient[0], m->ambient[1], m->ambient[2], m->ambient[3]);
		color col_dif(m->diffuse[0], m->diffuse[1], m->diffuse[2], m->diffuse[3]);
		color col_spec(m->specular[0], m->specular[1], m->specular[2], m->specular[3]);
		color col_emis(m->emissive[0], m->emissive[1], m->emissive[2], m->emissive[3]);

		material mat;
		mat.set_name(ms3d.mat[i].name);

		mat.set_color(col_amb, MATTR_AMBIENT);
		mat.set_color(col_dif, MATTR_DIFFUSE);
		mat.set_color(col_spec, MATTR_SPECULAR);
		mat.set_color(col_emis, MATTR_EMISSION);

		mat.set(m->shininess, MATTR_SHININESS);
		mat.set(1.0 - m->transparency, MATTR_ALPHA);

		mat.set_texture(get_texture(m->texture));

		matlib[ms3d.mat[i].name] = mat;
	}

	// construct robjects from the ms3d groups and add them to the scene
	for(int i=0; i<ms3d.num_grp; i++) {
		robject *obj = cons_object(&ms3d, ms3d.grp + i);

		char *mat_name = ms3d.mat[(int)ms3d.grp[i].matref].name;
		obj->set_material(matlib[mat_name]);

		add_object(obj);
	}

	return true;
}

static bool read_header(FILE *fp, ms3d_header *hdr)
{
	if(fread(hdr, sizeof *hdr, 1, fp) < 1) {
		error(UNEXP_EOF);
		return false;
	}

	hdr->magic[10] = 0;	// this overwrites fmt_ver, check it first if needed
	if(strcmp(hdr->magic, "MS3D000000") != 0) {
		return false;	// not a milkshape3d file
	}
	return true;
}


static ms3d_vertex *read_vertices(FILE *fp, int *count)
{
	ms3d_vertex *vert;
	uint16_t vcount;

	if(fread(&vcount, sizeof vcount, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		vert = new ms3d_vertex[vcount];
	}
	catch(...) {
		return 0;
	}

	if(fread(vert, sizeof *vert, vcount, fp) < vcount) {
		error(UNEXP_EOF);
		delete [] vert;
		return 0;
	}

	*count = vcount;
	return vert;
}


static ms3d_triangle *read_triangles(FILE *fp, int *count)
{
	ms3d_triangle *tri;
	uint16_t tcount;

	if(fread(&tcount, sizeof tcount, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		tri = new ms3d_triangle[tcount];
	}
	catch(...) {
		return 0;
	}

	if(fread(tri, sizeof *tri, tcount, fp) < tcount) {
		error(UNEXP_EOF);
		delete [] tri;
		return 0;
	}

	*count = tcount;
	return tri;
}


static ms3d_group *read_groups(FILE *fp, int *count)
{
	ms3d_group *grp;
	uint16_t grp_count;

	if(fread(&grp_count, sizeof grp_count, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		grp = new ms3d_group[grp_count];
	}
	catch(...) {
		return 0;
	}

	for(uint16_t i=0; i<grp_count; i++) {
		grp[i].flags = fgetc(fp);
		fread(grp[i].name, 1, sizeof grp[i].name, fp);
		fread(&grp[i].tri_count, sizeof grp[i].tri_count, 1, fp);

		if(feof(fp)) {
			error(UNEXP_EOF);
			delete [] grp;
			return 0;
		}

		grp[i].tri_idx = new uint16_t[grp[i].tri_count];
		fread(grp[i].tri_idx, sizeof *grp[i].tri_idx, grp[i].tri_count, fp);

		grp[i].matref = fgetc(fp);
		if(feof(fp)) {
			error(UNEXP_EOF);
			delete [] grp;
			return 0;
		}
	}

	*count = grp_count;
	return grp;
}

static ms3d_material *read_materials(FILE *fp, int *count)
{
	ms3d_material *mat;
	uint16_t mat_count;

	if(fread(&mat_count, sizeof mat_count, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		mat = new ms3d_material[mat_count];
	}
	catch(...) {
		return 0;
	}

	if(fread(mat, sizeof *mat, mat_count, fp) < mat_count) {
		error(UNEXP_EOF);
		delete [] mat;
		return 0;
	}

	*count = mat_count;
	return mat;
}

static robject *cons_object(const ms3d_file *ms3d, const ms3d_group *grp)
{
	robject *obj = 0;
	Vector3 *varr = 0;
	Vector3 *narr = 0;
	Vector2 *tarr = 0;

	int nelem = grp->tri_count * 3;

	try {
		obj = new robject;
		varr = new Vector3[nelem];
		narr = new Vector3[nelem];
		tarr = new Vector2[nelem];
	}
	catch(...) {
		delete obj;
		delete [] varr;
		delete [] narr;
		return false;
	}

	for(int i=0; i<grp->tri_count; i++) {
		for(int j=0; j<3; j++) {
			int idx = i * 3 + j;
			ms3d_triangle *tri = ms3d->tri + grp->tri_idx[i];

			varr[idx] = Vector3(ms3d->vert[tri->v[j]].pos[0],
					ms3d->vert[tri->v[j]].pos[1],
					ms3d->vert[tri->v[j]].pos[2]);
			narr[idx] = Vector3(tri->vnorm[j][0], tri->vnorm[j][1], tri->vnorm[j][2]);
			tarr[idx] = Vector2(tri->s[j], tri->t[j]);
		}
	}

	trimesh *mesh = obj->get_mesh();
	mesh->set_data(EL_VERTEX, varr, nelem);
	mesh->set_data(EL_NORMAL, narr, nelem);
	mesh->set_data(EL_TEXCOORD, tarr, nelem);

	delete [] varr;
	delete [] narr;
	delete [] tarr;
	return obj;
}


ms3d_group::ms3d_group()
{
	tri_idx = 0;
}

ms3d_group::~ms3d_group()
{
	delete [] tri_idx;
}

ms3d_file::ms3d_file()
{
	vert = 0;
	tri = 0;
	grp = 0;
	mat = 0;
}

ms3d_file::~ms3d_file()
{
	delete [] vert;
	delete [] tri;
	delete [] grp;
	delete [] mat;
}
