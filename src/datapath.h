#ifndef HENGE_DATAPATH_H_
#define HENGE_DATAPATH_H_

#include <limits.h>

#ifndef PATH_MAX
#if defined(WIN32) || defined(__WIN32__)
#define PATH_MAX	260
#endif	// win32
#endif

namespace henge {

bool set_path(const char *path);
bool add_path(const char *path);
const char *get_path();

bool find_file(const char *fname, char *pathname = 0, int pnsz = PATH_MAX);

}	// namespace henge

#endif	// HENGE_DATAPATH_H_
