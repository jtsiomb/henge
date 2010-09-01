#include <map>
#include <string>
#include "unicache.h"
#include "sdr.h"
#include "errlog.h"

enum uniform_type {
	UNI_INT,
	UNI_FLT,
	UNI_VEC,
	UNI_MAT
};

struct uniform {
	uniform_type type;

	int ival;
	float fval;
	Vector4 vec;
	Matrix4x4 mat;
};

using namespace std;
using namespace henge;

static map<string, uniform> uni_cache;

void henge::init_uniform_cache()
{
	// materials
	cache_uniform("uc_mat_roughness", 0.0f);
	cache_uniform("uc_mat_ior", 0.0f);

	// textures
	cache_uniform("uc_tex0", 0);
	cache_uniform("uc_tex1", 1);
	cache_uniform("uc_tex2", 2);
	cache_uniform("uc_tex3", 3);
	cache_uniform("uc_tex[0]", 0);
	cache_uniform("uc_tex[1]", 1);
	cache_uniform("uc_tex[2]", 2);
	cache_uniform("uc_tex[3]", 3);

	// lights
	cache_uniform("uc_num_lights", 1);

	for(int i=0; i<caps.max_lights; i++) {
		char uname[128];
		sprintf(uname, "uc_spot_inner[%d]", i);
		cache_uniform(uname, 0.0f);
		sprintf(uname, "uc_spot_outer[%d]", i);
		cache_uniform(uname, 0.0f);
		sprintf(uname, "uc_spot_inner%d", i);
		cache_uniform(uname, 0.0f);
		sprintf(uname, "uc_spot_outer%d", i);
		cache_uniform(uname, 0.0f);
	}
}

void henge::cache_uniform(const char *name, int val)
{
	uniform v;
	v.type = UNI_INT;
	v.ival = val;
	uni_cache[name] = v;
}

void henge::cache_uniform(const char *name, float val)
{
	uniform v;
	v.type = UNI_FLT;
	v.fval = val;
	uni_cache[name] = v;
}

void henge::cache_uniform(const char *name, const Vector4 &vec)
{
	uniform v;
	v.type = UNI_VEC;
	v.vec = vec;
	uni_cache[name] = v;
}

void henge::cache_uniform(const char *name, const Matrix4x4 &mat)
{
	uniform v;
	v.type = UNI_MAT;
	v.mat = mat;
	uni_cache[name] = v;
}

bool henge::is_cached_uniform(const char *name)
{
	map<string, uniform>::iterator iter = uni_cache.find(name);
	return iter != uni_cache.end();
}

bool henge::bind_cached_uniform(const char *name, const shader *sdr)
{
	map<string, uniform>::iterator iter = uni_cache.find(name);
	if(iter == uni_cache.end()) {
		return false;
	}

	uniform v = iter->second;
	switch(v.type) {
	case UNI_INT:
		//info("%s = %d\n", name, v.ival);
		return sdr->set_uniform(name, v.ival);

	case UNI_FLT:
		//info("%s = %f\n", name, v.fval);
		return sdr->set_uniform(name, v.fval);

	case UNI_VEC:
		return sdr->set_uniform(name, v.vec);

	case UNI_MAT:
		return sdr->set_uniform(name, v.mat);
	}

	return false;	// unreachable
}
