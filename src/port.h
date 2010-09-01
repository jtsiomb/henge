// portability wrappers (actually implementation of UNIX stuff on windows)
#ifndef HENGE_PORT_H_
#define HENGE_PORT_H_

#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(__WIN32__)

#include <malloc.h>
#include <direct.h>
#include <io.h>

#define stat(path, sbuf)	_stat(path, sbuf)
#define mkdir(path, mode)	_mkdir(path)
#define access(path, mode)	_access(path, mode)

#ifndef F_OK
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4
#endif

#else

#include <unistd.h>
#include <alloca.h>

#endif

#endif	// HENGE_PORT_H_
