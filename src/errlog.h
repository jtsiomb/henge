#ifndef HENGE_ERRLOG_H_
#define HENGE_ERRLOG_H_

#include <stdio.h>

namespace henge {

enum {
	LOG_INFO	= 1,
	LOG_WARNING	= 2,
	LOG_ERROR	= 4,

	LOG_ALL		= 7
};

bool set_log_file(const char *fname, unsigned int mtypes);
bool set_log_stream(FILE *fp, unsigned int mtypes);
bool set_log_callback(void (*func)(const char*), unsigned int mtypes);

void info(const char *str, ...);
void warning(const char *str, ...);
void error(const char *str, ...);

}

#endif	// HENGE_ERRLOG_H_
