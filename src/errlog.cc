#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "errlog.h"

using namespace std;
using namespace henge;

class log_target {
protected:
	unsigned int msg_mask;

public:
	log_target(unsigned int mask = 0);
	virtual ~log_target();

	virtual void log(const char *msg, unsigned int type) = 0;
};

class log_file : public log_target {
protected:
	char *fname;
	bool first_time;

public:
	log_file(const char *fname, unsigned int mask);
	virtual ~log_file();

	virtual void log(const char *msg, unsigned int type);
};

class log_stream : public log_target {
protected:
	FILE *fp;

public:
	log_stream(FILE *fp, unsigned int mask);
	virtual ~log_stream();

	virtual void log(const char *msg, unsigned int type);
};

class log_callback : public log_target {
protected:
	void (*callback)(const char*);

public:
	log_callback(void (*cbfunc)(const char*), unsigned int mask);
	virtual ~log_callback();

	virtual void log(const char *msg, unsigned int type);
};

static vector<log_target*> log_list;

#ifdef _MSC_VER
#define vsnprintf(str, size, format, ap)	_vsnprintf(str, size, format, ap)
#endif

static bool atexit_set;

static void cleanup()
{
	for(size_t i=0; i<log_list.size(); i++) {
		delete log_list[i];
	}
	log_list.clear();
}

bool henge::set_log_file(const char *fname, unsigned int mtypes)
{
	if(!atexit_set) {
		atexit(cleanup);
		atexit_set = true;
	}

	log_file *log;
	try {
		log = new log_file(fname, mtypes);
		log_list.push_back(log);
	}
	catch(...) {
		return false;
	}
	return true;
}

bool henge::set_log_stream(FILE *fp, unsigned int mtypes)
{
	if(!atexit_set) {
		atexit(cleanup);
		atexit_set = true;
	}

	log_stream *log;
	try {
		log = new log_stream(fp, mtypes);
		log_list.push_back(log);
	}
	catch(...) {
		return false;
	}
	return true;
}

bool henge::set_log_callback(void (*func)(const char*), unsigned int mtypes)
{
	if(!atexit_set) {
		atexit(cleanup);
		atexit_set = true;
	}

	log_callback *log;
	try {
		log = new log_callback(func, mtypes);
		log_list.push_back(log);
	}
	catch(...) {
		return false;
	}
	return true;
}

void henge::info(const char *str, ...)
{
	va_list ap;
	char msg_buf[2048];

	va_start(ap, str);
	vsnprintf(msg_buf, sizeof msg_buf, str, ap);
	va_end(ap);

	for(size_t i=0; i<log_list.size(); i++) {
		log_list[i]->log(msg_buf, LOG_INFO);
	}
}

void henge::warning(const char *str, ...)
{
	va_list ap;
	char msg_buf[2048];

	va_start(ap, str);
	vsnprintf(msg_buf, sizeof msg_buf, str, ap);
	va_end(ap);

	for(size_t i=0; i<log_list.size(); i++) {
		log_list[i]->log(msg_buf, LOG_WARNING);
	}
}

void henge::error(const char *str, ...)
{
	va_list ap;
	char msg_buf[2048];

	va_start(ap, str);
	vsnprintf(msg_buf, sizeof msg_buf, str, ap);
	va_end(ap);

	for(size_t i=0; i<log_list.size(); i++) {
		log_list[i]->log(msg_buf, LOG_ERROR);
	}
}

log_target::log_target(unsigned int mask)
{
	msg_mask = mask;
}

log_target::~log_target() {}

log_file::log_file(const char *fname, unsigned int mask)
	: log_target(mask)
{
	this->fname = new char[strlen(fname) + 1];
	strcpy(this->fname, fname);
	first_time = true;
}

log_file::~log_file() 
{
	delete [] fname;
}

void log_file::log(const char *msg, unsigned int type)
{
	FILE *fp;

	if(!(type & msg_mask)) {
		return;
	}

	if(!(fp = fopen(fname, "a"))) {
		if(first_time) {
			first_time = false;
			fprintf(stderr, "failed to open log file: %s: %s", fname, strerror(errno));
		}
		return;
	}
	fputs(msg, fp);
	fclose(fp);
}

log_stream::log_stream(FILE *fp, unsigned int mask)
	: log_target(mask)
{
	this->fp = fp;
}

log_stream::~log_stream() {}

void log_stream::log(const char *msg, unsigned int type)
{
	if(type & msg_mask) {
#if defined(unix) || defined(__unix__)
		if(isatty(fileno(fp)) && type != LOG_INFO) {
			int c = type == LOG_ERROR ? 31 : 33;
			fprintf(fp, "\033[%dm%s\033[0m", c, msg);
		} else
#endif
		{
			fputs(msg, fp);
		}
		if(type == LOG_ERROR) {
			fflush(fp);
		}
	}
}

log_callback::log_callback(void (*cbfunc)(const char*), unsigned int mask)
	: log_target(mask)
{
	callback = cbfunc;
}

log_callback::~log_callback() {}

void log_callback::log(const char *msg, unsigned int type)
{
	if(type & msg_mask) {
		callback(msg);
	}
}
