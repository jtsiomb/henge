#ifndef HENGE_UNIFORM_CACHE_H_
#define HENGE_UNIFORM_CACHE_H_

#include <map>
#include <string>
#include <vmath.h>

namespace henge {

class shader;

void init_uniform_cache();

void cache_uniform(const char *name, int val);
void cache_uniform(const char *name, float val);
void cache_uniform(const char *name, const Vector4 &vec);
void cache_uniform(const char *name, const Matrix4x4 &mat);

bool is_cached_uniform(const char *name);

bool bind_cached_uniform(const char *name, const shader *sdr);

}

#endif	// HENGE_UNIFORM_CACHE_H_
