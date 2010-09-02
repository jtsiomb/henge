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
struct MS3DHeader {
	char magic[10];
	int fmt_ver;
} PACKED;

struct MS3DVertex {
	char flags;
	float pos[3];
	char bone_id;		// -1 means no bone
	char ref_count;
} PACKED;

struct MS3DTriangle {
	uint16_t flags;
	uint16_t v[3];
	float vnorm[3][3];
	float s[3];
	float t[3];
	unsigned char smoothing_group;
	unsigned char group_idx;
} PACKED;

struct MS3DMaterial {
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

class MS3DGroup {
public:
	char flags;
	char name[32];
	uint16_t tri_count;
	uint16_t *tri_idx;
	char matref;

	MS3DGroup();
	~MS3DGroup();
};

class MS3DFile {
public:
	MS3DHeader hdr;
	MS3DVertex *vert;
	MS3DTriangle *tri;
	MS3DGroup *grp;
	MS3DMaterial *mat;
	int num_vert, num_tri, num_grp, num_mat;

	MS3DFile();
	~MS3DFile();
};

#define UNEXP_EOF	"load_ms3d: premature end of file encountered\n"

static bool read_header(FILE *fp, MS3DHeader *hdr);
static MS3DVertex *read_vertices(FILE *fp, int *count);
static MS3DTriangle *read_triangles(FILE *fp, int *count);
static MS3DGroup *read_groups(FILE *fp, int *count);
static MS3DMaterial *read_materials(FILE *fp, int *count);
static RObject *cons_object(const MS3DFile *ms3d, const MS3DGroup *grp);

static map<string, Material> matlib;

bool Scene::load_ms3d(FILE *fp)
{
	MS3DFile ms3d;

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
		MS3DMaterial *m = ms3d.mat + i;

		Color col_amb(m->ambient[0], m->ambient[1], m->ambient[2], m->ambient[3]);
		Color col_dif(m->diffuse[0], m->diffuse[1], m->diffuse[2], m->diffuse[3]);
		Color col_spec(m->specular[0], m->specular[1], m->specular[2], m->specular[3]);
		Color col_emis(m->emissive[0], m->emissive[1], m->emissive[2], m->emissive[3]);

		Material mat;
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
		RObject *obj = cons_object(&ms3d, ms3d.grp + i);

		char *mat_name = ms3d.mat[(int)ms3d.grp[i].matref].name;
		obj->set_material(matlib[mat_name]);

		add_object(obj);
	}

	return true;
}

static bool read_header(FILE *fp, MS3DHeader *hdr)
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


static MS3DVertex *read_vertices(FILE *fp, int *count)
{
	MS3DVertex *vert;
	uint16_t vcount;

	if(fread(&vcount, sizeof vcount, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		vert = new MS3DVertex[vcount];
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


static MS3DTriangle *read_triangles(FILE *fp, int *count)
{
	MS3DTriangle *tri;
	uint16_t tcount;

	if(fread(&tcount, sizeof tcount, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		tri = new MS3DTriangle[tcount];
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


static MS3DGroup *read_groups(FILE *fp, int *count)
{
	MS3DGroup *grp;
	uint16_t grp_count;

	if(fread(&grp_count, sizeof grp_count, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		grp = new MS3DGroup[grp_count];
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

static MS3DMaterial *read_materials(FILE *fp, int *count)
{
	MS3DMaterial *mat;
	uint16_t mat_count;

	if(fread(&mat_count, sizeof mat_count, 1, fp) < 1) {
		error(UNEXP_EOF);
		return 0;
	}

	try {
		mat = new MS3DMaterial[mat_count];
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

static RObject *cons_object(const MS3DFile *ms3d, const MS3DGroup *grp)
{
	RObject *obj = 0;
	Vector3 *varr = 0;
	Vector3 *narr = 0;
	Vector2 *tarr = 0;

	int nelem = grp->tri_count * 3;

	try {
		obj = new RObject;
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
			MS3DTriangle *tri = ms3d->tri + grp->tri_idx[i];

			varr[idx] = Vector3(ms3d->vert[tri->v[j]].pos[0],
					ms3d->vert[tri->v[j]].pos[1],
					ms3d->vert[tri->v[j]].pos[2]);
			narr[idx] = Vector3(tri->vnorm[j][0], tri->vnorm[j][1], tri->vnorm[j][2]);
			tarr[idx] = Vector2(tri->s[j], tri->t[j]);
		}
	}

	TriMesh *mesh = obj->get_mesh();
	mesh->set_data(EL_VERTEX, varr, nelem);
	mesh->set_data(EL_NORMAL, narr, nelem);
	mesh->set_data(EL_TEXCOORD, tarr, nelem);

	delete [] varr;
	delete [] narr;
	delete [] tarr;
	return obj;
}


MS3DGroup::MS3DGroup()
{
	tri_idx = 0;
}

MS3DGroup::~MS3DGroup()
{
	delete [] tri_idx;
}

MS3DFile::MS3DFile()
{
	vert = 0;
	tri = 0;
	grp = 0;
	mat = 0;
}

MS3DFile::~MS3DFile()
{
	delete [] vert;
	delete [] tri;
	delete [] grp;
	delete [] mat;
}
