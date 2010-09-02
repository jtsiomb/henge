#include <assert.h>
#include <float.h>
#include "opengl.h"
#include "mesh.h"
#include "sdr.h"
#include "kdtree.h"
#include "errlog.h"

/* Define DEBUG_DRAWING to force drawing of meshes through immediate mode calls.
 * Sometimes it's helpful to see index indirections etc happen in the cpu directly
 * easier to spot&debug fucked up index buffers, missing attribute buffers, etc.
 */
#undef DEBUG_DRAWING

#ifdef DEBUG_DRAWING
#warning "DEBUG_DRAWING is defined in mesh.cc, drawing in immediate mode"
#endif

using namespace henge;

TriMesh::TriMesh()
{
	init();
}

TriMesh::TriMesh(const TriMesh &m)
{
	init();
	*this = m;
}

void TriMesh::init()
{
	vert = norm = tang = 0;
	tc = 0;
	col = 0;
	index = 0;
	nvert = nnorm = ntang = ntc = ncol = nindex = 0;

	dynamic = false;
	dlist = 0;

	memset(vbo, 0, EL_COUNT * sizeof *vbo);

	if(caps.vbo) {
		glGenBuffersARB(EL_COUNT, vbo);
		memset(vbo_valid, 0, EL_COUNT * sizeof *vbo_valid);
	}

	kdt_valid = false;
	bounds_valid = false;
}

#define ELEM_BIT(x)		(1 << x)
void TriMesh::invalidate(int elmask)
{
	if(dlist) {
		glDeleteLists(dlist, 1);
		dlist = 0;
	}

	for(int i=0; i<EL_COUNT; i++) {
		if(elmask & (1 << i)) {
			vbo_valid[i] = false;
		}
	}
}

TriMesh &TriMesh::operator =(const TriMesh &m)
{
	if(this == &m) {
		return *this;
	}
	nvert = m.nvert;
	nnorm = m.nnorm;
	ntang = m.ntang;
	ntc = m.ntc;
	ncol = m.ncol;
	nindex = m.nindex;

	dynamic = m.dynamic;

	// copy the various arrays
	if(m.vert) {
		vert = new Vector3[nvert];
		memcpy(vert, m.vert, nvert * sizeof *vert);
	}
	if(m.norm) {
		norm = new Vector3[nnorm];
		memcpy(norm, m.norm, nnorm * sizeof *norm);
	}
	if(m.tang) {
		tang = new Vector3[ntang];
		memcpy(tang, m.tang, ntang * sizeof *tang);
	}
	if(m.tc) {
		tc = new Vector2[ntc];
		memcpy(tc, m.tc, ntc * sizeof *tc);
	}
	if(m.col) {
		col = new Vector4[ncol];
		memcpy(col, m.col, ncol * sizeof *col);
	}
	if(m.index) {
		index = new unsigned int[nindex];
		memcpy(index, m.index, nindex * sizeof *index);
	}

	return *this;
}

TriMesh::~TriMesh()
{
	delete [] vert;
	delete [] norm;
	delete [] tang;
	delete [] tc;
	delete [] col;
	delete [] index;

	if(dlist) {
		glDeleteLists(dlist, 1);
	}

	if(vbo[0]) {
		glDeleteBuffersARB(EL_COUNT, vbo);
	}
}

void TriMesh::set_dynamic(bool dynamic)
{
	this->dynamic = dynamic;
}

bool TriMesh::get_dynamic() const
{
	return dynamic;
}

bool TriMesh::merge(const TriMesh &mesh)
{
	int vidx_offs = 0;

	if((index && !mesh.index) || (!index && mesh.index)) {
		error("merging of an indexed with a non-indexed mesh not supported yet");
		return false;
	}

	// merge positions
	if(vert || mesh.vert) {
		Vector3 *new_vert = new Vector3[nvert + mesh.nvert];
		memcpy(new_vert, vert, nvert * sizeof *new_vert);
		memcpy(new_vert + nvert, mesh.vert, mesh.nvert * sizeof *new_vert);
		vidx_offs = nvert;

		delete [] vert;
		vert = new_vert;
		nvert += mesh.nvert;
		kdt_valid = false;
		bounds_valid = false;

		invalidate(ELEM_BIT(EL_VERTEX));
	}

	// merge normals
	if(norm || mesh.norm) {
		Vector3 *new_norm = new Vector3[nnorm + mesh.nnorm];
		memcpy(new_norm, norm, nnorm * sizeof *new_norm);
		memcpy(new_norm + nnorm, mesh.norm, mesh.nnorm * sizeof *new_norm);

		delete [] norm;
		norm = new_norm;
		nnorm += mesh.nnorm;
		invalidate(ELEM_BIT(EL_NORMAL));
	}

	// merge tangents
	if(tang || mesh.tang) {
		Vector3 *new_tang = new Vector3[ntang + mesh.ntang];
		memcpy(new_tang, tang, ntang * sizeof *new_tang);
		memcpy(new_tang + ntang, mesh.tang, mesh.ntang * sizeof *new_tang);

		delete [] tang;
		tang = new_tang;
		ntang += mesh.ntang;
		invalidate(ELEM_BIT(EL_TANGENT));
	}

	// merge tex coords
	if(tc || mesh.tc) {
		Vector2 *new_tc = new Vector2[ntc + mesh.ntc];
		memcpy(new_tc, tc, ntc * sizeof *new_tc);
		memcpy(new_tc + ntc, mesh.tc, mesh.ntc * sizeof *new_tc);

		delete [] tc;
		tc = new_tc;
		ntc += mesh.ntc;
		invalidate(ELEM_BIT(EL_TEXCOORD));
	}

	if(col || mesh.col) {
		Vector4 *new_col = new Vector4[ncol + mesh.ncol];
		memcpy(new_col, col, ncol * sizeof *new_col);
		memcpy(new_col + ncol, mesh.col, mesh.ncol * sizeof *new_col);

		delete [] col;
		col = new_col;
		ncol += mesh.ncol;
		invalidate(ELEM_BIT(EL_COLOR));
	}

	// merge indices
	if(index && mesh.index) {
		unsigned int *new_index = new unsigned int[nindex + mesh.nindex];
		memcpy(new_index, index, nindex * sizeof *new_index);

		unsigned int *iptr = new_index + nindex;
		for(int i=0; i<mesh.nindex; i++) {
			*iptr++ = mesh.index[i] + vidx_offs;
		}

		delete [] index;
		index = new_index;
		nindex += mesh.nindex;
		invalidate(ELEM_BIT(EL_INDEX));
	}

	if(dlist) {
		glDeleteLists(1, dlist);
		dlist = 0;
	}
	return true;
}

bool TriMesh::set_data(int elem, const Vector4 *data, int count)
{
	if(elem != EL_COLOR) {
		return false;
	}

	Vector4 *carr;
	try {
		carr = new Vector4[count];
	}
	catch(...) {
		return false;
	}

	col = carr;

	if(data) {
		memcpy(col, data, count * sizeof *col);
	}
	ncol = count;

	invalidate(ELEM_BIT(elem));
	return true;
}

bool TriMesh::set_data(int elem, const Vector3 *data, int count)
{
	Vector3 *varr;
	try {
		varr = new Vector3[count];
	}
	catch(...) {
		return false;
	}

	switch(elem) {
	case EL_VERTEX:
		delete [] vert;
		vert = varr;
		nvert = count;
		kdt_valid = false;
		bounds_valid = false;
		break;

	case EL_NORMAL:
		delete [] norm;
		norm = varr;
		nnorm = count;
		break;

	case EL_TANGENT:
		delete [] tang;
		tang = varr;
		ntang = count;
		break;

	default:
		delete [] varr;
		return false;
	}

	if(data) {
		memcpy(varr, data, count * sizeof *varr);
	}

	invalidate(ELEM_BIT(elem));
	return true;
}

bool TriMesh::set_data(int elem, const Vector2 *data, int count)
{
	if(elem != EL_TEXCOORD) {
		return false;
	}

	try {
		Vector2 *new_tc = new Vector2[count];
		delete [] tc;
		tc = new_tc;
	}
	catch(...) {
		return false;
	}

	if(data) {
		memcpy(tc, data, count * sizeof *tc);
	}
	ntc = count;

	invalidate(ELEM_BIT(elem));
	return true;
}

bool TriMesh::set_data(int elem, const unsigned int *data, int count)
{
	if(elem != EL_INDEX) {
		return false;
	}

	try {
		unsigned int *idx = new unsigned int[count];
		delete [] index;
		index = idx;
	}
	catch(...) {
		return false;
	}

	if(data) {
		memcpy(index, data, count * sizeof *index);
	}
	nindex = count;

	kdt_valid = false;

	invalidate(ELEM_BIT(elem));
	return true;
}

Vector4 *TriMesh::get_data_vec4(int elem)
{
	if(elem == EL_COLOR) {
		if(col) {
			invalidate(ELEM_BIT(EL_COLOR));
		}
		return col;
	}
	return 0;
}

const Vector4 *TriMesh::get_data_vec4(int elem) const
{
	if(elem == EL_COLOR) {
		return col;
	}
	return 0;
}

Vector3 *TriMesh::get_data_vec3(int elem)
{
	switch(elem) {
	case EL_VERTEX:
		kdt_valid = false;
		if(vert) {
			invalidate(ELEM_BIT(EL_VERTEX));
		}
		return vert;

	case EL_NORMAL:
		if(norm) {
			invalidate(ELEM_BIT(EL_NORMAL));
		}
		return norm;

	case EL_TANGENT:
		if(tang) {
			invalidate(ELEM_BIT(EL_TANGENT));
		}
		return tang;

	default:
		break;
	}
	return 0;
}

const Vector3 *TriMesh::get_data_vec3(int elem) const
{
	switch(elem) {
	case EL_VERTEX:
		return vert;

	case EL_NORMAL:
		return norm;

	case EL_TANGENT:
		return tang;

	default:
		break;
	}
	return 0;
}

Vector2 *TriMesh::get_data_vec2(int elem)
{
	if(elem == EL_TEXCOORD) {
		if(tc) {
			invalidate(ELEM_BIT(EL_TEXCOORD));
		}
		return tc;
	}
	return 0;
}

const Vector2 *TriMesh::get_data_vec2(int elem) const
{
	if(elem == EL_TEXCOORD) {
		return tc;
	}
	return 0;
}

unsigned int *TriMesh::get_data_int(int elem)
{
	if(elem == EL_INDEX) {
		if(index) {
			invalidate(ELEM_BIT(EL_INDEX));
			kdt_valid = false;
		}
		return index;
	}
	return 0;
}

const unsigned int *TriMesh::get_data_int(int elem) const
{
	if(elem == EL_INDEX) {
		return index;
	}
	return 0;
}

int TriMesh::get_count(int elem) const
{
	switch(elem) {
	case EL_VERTEX:
		return nvert;

	case EL_NORMAL:
		return nnorm;

	case EL_TANGENT:
		return ntang;

	case EL_TEXCOORD:
		return ntc;

	case EL_COLOR:
		return ncol;

	case EL_INDEX:
		return nindex;
	}
	return 0;
}


void TriMesh::build_kdtree()
{
	kdt.clear();

	for(int i=0; i<nvert; i++) {
		// XXX last argument is Triangle index... won't work for indexed Triangles
		kdt.insert(vert[i].x, vert[i].y, vert[i].z, i / 3);
	}
	kdt_valid = true;
}

void TriMesh::calc_normals()
{
	// TODO implement
}

void TriMesh::indexify(float threshold)
{
	/*if(index) return;	// already indexed

	int num_tri = nvert / 3;
	set_data(EL_INDEX, 0, num_tri * 3);

	vector<Vector3> varr;

	for(int i=0; i<nvert; i++) {

	}*/
}

#define SWAP(type, a, b)	\
	do {					\
		type tmp = a;		\
		a = b;				\
		b = tmp;			\
	} while(0)


void TriMesh::flip_winding()
{
	if(index) {
		for(int i=0; i<nindex; i++) {
			if(i % 3 == 2) {
				SWAP(int, index[i], index[i - 2]);
			}
		}
		// invalidate index buffer object
		invalidate(ELEM_BIT(EL_INDEX));
	} else {
		for(int i=0; i<nvert; i++) {
			if(i % 3 == 2) {
				if(vert) {
					SWAP(Vector3, vert[i], vert[i - 2]);
				}
				if(norm) {
					assert(nvert == nnorm);
					SWAP(Vector3, norm[i], norm[i - 2]);
				}
				if(tang) {
					assert(nvert == ntang);
					SWAP(Vector3, tang[i], tang[i - 2]);
				}
				if(tc) {
					assert(nvert == ntc);
					SWAP(Vector2, tc[i], tc[i - 2]);
				}
				if(col) {
					assert(nvert == ncol);
					SWAP(Vector4, col[i], col[i - 2]);
				}
			}
		}
		// invalidate all changed vbos
		invalidate(ELEM_BIT(EL_VERTEX) | ELEM_BIT(EL_NORMAL) | ELEM_BIT(EL_TANGENT) |
				ELEM_BIT(EL_TEXCOORD) | ELEM_BIT(EL_COLOR));
	}
}

void TriMesh::flip_normals()
{
	for(int i=0; i<nvert; i++) {
		norm[i] = -norm[i];
	}
	invalidate(ELEM_BIT(EL_NORMAL));
}

void TriMesh::transform(const Matrix4x4 &mat)
{
	Matrix4x4 norm_mat;

	for(int i=0; i<nvert; i++) {
		vert[i].transform(mat);
	}
	invalidate(ELEM_BIT(EL_VERTEX));
	bounds_valid = false;

	if(norm || tang) {
		norm_mat = mat.inverse().transposed();
	}

	if(norm) {
		for(int i=0; i<nnorm; i++) {
			norm[i].transform(norm_mat);
		}
		invalidate(ELEM_BIT(EL_NORMAL));
	}
	if(tang) {
		for(int i=0; i<ntang; i++) {
			tang[i].transform(norm_mat);
		}
		invalidate(ELEM_BIT(EL_TANGENT));
	}
}

Vector3 TriMesh::get_centroid() const
{
	if(!bounds_valid) {
		((TriMesh*)this)->calc_bounds();
	}
	return centroid;
}

Vector3 TriMesh::get_aabb_min() const
{
	if(!bounds_valid) {
		((TriMesh*)this)->calc_bounds();
	}
	return aabb_min;
}

Vector3 TriMesh::get_aabb_max() const
{
	if(!bounds_valid) {
		((TriMesh*)this)->calc_bounds();
	}
	return aabb_max;
}

float TriMesh::get_bsph_radius() const
{
	if(!bounds_valid) {
		((TriMesh*)this)->calc_bounds();
	}
	return bsph_rad;
}

/* TODO optimize updates, use glBufferSubDataARB instead of glBufferDataARB */
void TriMesh::setup_vertex_arrays() const
{
#ifdef SINGLE_PRECISION_MATH
	static const GLenum gltype = GL_FLOAT;
#else
	static const GLenum gltype = GL_DOUBLE;
#endif

	// do we have vertices? (we better have some...)
	if(vert) {
		glEnableClientState(GL_VERTEX_ARRAY);

		if(caps.vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[EL_VERTEX]);

			// if it's invalid, update it
			if(!vbo_valid[EL_VERTEX]) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, nvert * sizeof *vert, vert, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_VERTEX] = true;
			}
			glVertexPointer(3, gltype, 0, 0);
		} else {
			glVertexPointer(3, gltype, 0, vert);
		}
	}

	// do we have normals?
	if(norm) {
		glEnableClientState(GL_NORMAL_ARRAY);

		if(caps.vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[EL_NORMAL]);

			// if it's invalid, update it
			if(!vbo_valid[EL_NORMAL]) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, nnorm * sizeof *norm, norm, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_NORMAL] = true;
			}
			glNormalPointer(gltype, 0, 0);
		} else {
			glNormalPointer(gltype, 0, norm);
		}
	}

	// do we have texture coordinates?
	if(tc) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		if(caps.vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[EL_TEXCOORD]);

			// if it's invalid, update it
			if(!vbo_valid[EL_TEXCOORD]) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, ntc * sizeof *tc, tc, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_TEXCOORD] = true;
			}
			glTexCoordPointer(2, gltype, 0, 0);
		} else {
			glTexCoordPointer(2, gltype, 0, tc);
		}
	}

	// do we have colors?
	if(col) {
		glEnableClientState(GL_COLOR_ARRAY);

		if(caps.vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[EL_COLOR]);

			// if it's invalid, update it
			if(!vbo_valid[EL_COLOR]) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, ncol * sizeof *col, col, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_COLOR] = true;
			}
			glColorPointer(4, gltype, 0, 0);
		} else {
			glColorPointer(4, gltype, 0, col);
		}
	}

	// do we have tangents?
	if(tang && caps.glsl) {
		glEnableVertexAttribArrayARB(SDR_ATTR_TANGENT);

		if(caps.vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[EL_TANGENT]);

			// if it's invalid, update it
			if(!vbo_valid[EL_TANGENT]) {
				glBufferDataARB(GL_ARRAY_BUFFER_ARB, ntang * sizeof *tang, tang, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_TANGENT] = true;
			}
			glVertexAttribPointerARB(SDR_ATTR_TANGENT, 3, gltype, 0, 0, 0);
		} else {
			glVertexAttribPointerARB(SDR_ATTR_TANGENT, 3, gltype, 0, 0, tang);
		}
	}

	if(caps.vbo) {
		// restore default binding
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
}


#ifndef DEBUG_DRAWING

void TriMesh::draw() const
{
	// if we've got a display list, just use it...
	if(dlist) {
		glCallList(dlist);
		return;
	}

	// if the mesh is NOT dynamic, then start compiling a display list
	if(!dynamic) {
		dlist = glGenLists(1);
		glNewList(dlist, GL_COMPILE);
	}

	setup_vertex_arrays();

	if(index) {		// indexed Triangles?
		if(caps.vbo) {
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo[EL_INDEX]);

			// update if needed
			if(!vbo_valid[EL_INDEX]) {
				glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nindex * sizeof *index,
						index, GL_DYNAMIC_DRAW_ARB);
				vbo_valid[EL_INDEX] = true;
			}

			glDrawElements(GL_TRIANGLES, nindex, GL_UNSIGNED_INT, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		} else {
			glDrawElements(GL_TRIANGLES, nindex, GL_UNSIGNED_INT, index);
		}

	} else {
		// raw vertex data, group them by 3 and draw
		glDrawArrays(GL_TRIANGLES, 0, nvert);
	}

	// disable all possible vertex arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	if(caps.glsl) {
		glDisableVertexAttribArrayARB(SDR_ATTR_TANGENT);
	}

	if(!dynamic) {
		glEndList();
		glCallList(dlist);
	}
}

#else

void TriMesh::draw() const
{
	glBegin(GL_TRIANGLES);
	if(index) {
		for(int i=0; i<nindex; i++) {
			int idx = index[i];
			if(norm) glNormal3f(norm[idx].x, norm[idx].y, norm[idx].z);
			if(tang) glVertexAttrib3f(SDR_ATTR_TANGENT, tang[idx].x, tang[idx].y, tang[idx].z);
			if(tc) glTexCoord2f(tc[idx].x, tc[idx].y);
			if(col) glColor4f(col[idx].x, col[idx].y, col[idx].z, col[idx].w);
			if(vert) glVertex3f(vert[idx].x, vert[idx].y, vert[idx].z);
		}
	} else {
		for(int i=0; i<nvert; i++) {
			if(norm) glNormal3f(norm[i].x, norm[i].y, norm[i].z);
			if(tang) glVertexAttrib3f(SDR_ATTR_TANGENT, tang[i].x, tang[i].y, tang[i].z);
			if(tc) glTexCoord2f(tc[i].x, tc[i].y);
			if(col) glColor4f(col[i].x, col[i].y, col[i].z, col[i].w);
			if(vert) glVertex3f(vert[i].x, vert[i].y, vert[i].z);
		}
	}
	glEnd();
}
#endif

void TriMesh::calc_bounds()
{
	centroid = Vector3(0, 0, 0);
	aabb_min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb_max = Vector3(FLT_MIN, FLT_MIN, FLT_MIN);
	bsph_rad = 0.0f;

	for(int i=0; i<nvert; i++) {
		centroid += vert[i];
		aabb_min.x = MIN(aabb_min.x, vert[i].x);
		aabb_max.x = MAX(aabb_max.x, vert[i].x);
		aabb_min.y = MIN(aabb_min.y, vert[i].y);
		aabb_max.y = MAX(aabb_max.y, vert[i].y);
		aabb_min.z = MIN(aabb_min.z, vert[i].z);
		aabb_max.z = MAX(aabb_max.z, vert[i].z);
	}
	centroid /= (float)nvert;

	for(int i=0; i<nvert; i++) {
		float dist = (vert[i] - centroid).length();
		if(dist > bsph_rad) {
			bsph_rad = dist;
		}
	}

	bounds_valid = true;
}


void TriMesh::draw_normals(float sz, const Color &col) const
{
	if(!norm) return;

	glBegin(GL_LINES);
	glColor4f(col.x, col.y, col.z, col.w);
	for(int i=0; i<nvert; i++) {
		float x = vert[i].x;
		float y = vert[i].y;
		float z = vert[i].z;

		glVertex3f(x, y, z);
		glVertex3f(x + norm[i].x * sz, y + norm[i].y * sz, z + norm[i].z * sz);
	}
	glEnd();
}


void TriMesh::draw_tangents(float sz, const Color &col) const
{
	if(!tang) return;

	glBegin(GL_LINES);
	glColor4f(col.x, col.y, col.z, col.w);
	for(int i=0; i<nvert; i++) {
		float x = vert[i].x;
		float y = vert[i].y;
		float z = vert[i].z;

		glVertex3f(x, y, z);
		glVertex3f(x + tang[i].x * sz, y + tang[i].y * sz, z + tang[i].z * sz);
	}
	glEnd();
}

void TriMesh::draw_vertices(float sz, const Color &col) const
{
	glPushAttrib(GL_POINT_BIT);

	glEnable(GL_POINT_SMOOTH);
	glPointSize(sz);

	glBegin(GL_POINTS);
	glColor4f(col.x, col.y, col.z, col.w);
	for(int i=0; i<nvert; i++) {
		glVertex3f(vert[i].x, vert[i].y, vert[i].z);
	}
	glEnd();
	glPopAttrib();
}

struct Triangle {
	Vector3 v[3];
	Vector3 n;
};

static bool ray_triangle(const Ray &ray, Triangle *tri, float *pt);

bool TriMesh::intersect(const Ray &ray, float *pt) const
{
	float t0 = FLT_MAX;

	// indexed Triangles
	int ntris = index ? nindex / 3 : nvert / 3;
	for(int i=0; i<ntris; i++) {
		Triangle tri;

		if(index) {
			tri.v[0] = vert[index[i * 3]];
			tri.v[1] = vert[index[i * 3 + 1]];
			tri.v[2] = vert[index[i * 3 + 2]];
		} else {
			tri.v[0] = vert[i * 3];
			tri.v[1] = vert[i * 3 + 1];
			tri.v[2] = vert[i * 3 + 2];
		}

		Vector3 v1 = tri.v[1] - tri.v[0];
		Vector3 v2 = tri.v[2] - tri.v[0];
		tri.n = cross_product(v1, v2).normalized();

		float t;
		if(ray_triangle(ray, &tri, &t) && t < t0) {
			t0 = t;
		}
	}

	if(t0 < FLT_MAX) {
		if(pt) *pt = t0;
		return true;
	}
	return false;
}

static bool ray_triangle(const Ray &ray, Triangle *tri, float *pt)
{
	float vdotn = dot_product(ray.dir, tri->n);
	if(fabs(vdotn) < ERROR_MARGIN) {
		return false;	// parallel to the plane
	}

	float t = dot_product(tri->v[0] - ray.origin, tri->n) / vdotn;
	if(t < ERROR_MARGIN || t > 1.0) {
		return false;
	}

	Vector3 pos = ray.origin + ray.dir * t;

	for(int i=0; i<3; i++) {
		Vector3 v0 = tri->v[(i + 1) % 3] - tri->v[i];
		Vector3 v1 = pos - tri->v[i];

		Vector3 n = cross_product(v0, v1);
		if(dot_product(n, tri->n) < 0.0) {
			return false;
		}
	}

	if(pt) {
		*pt = t;
	}
	return true;
}
