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

class LogTarget {
protected:
	unsigned int msg_mask;

public:
	LogTarget(unsigned int mask = 0);
	virtual ~LogTarget();

	virtual void log(const char *msg, unsigned int type) = 0;
};

class LogFile : public LogTarget {
protected:
	char *fname;
	bool first_time;

public:
	LogFile(const char *fname, unsigned int mask);
	virtual ~LogFile();

	virtual void log(const char *msg, unsigned int type);
};

class LogStream : public LogTarget {
protected:
	FILE *fp;

public:
	LogStream(FILE *fp, unsigned int mask);
	virtual ~LogStream();

	virtual void log(const char *msg, unsigned int type);
};

class LogCallback : public LogTarget {
protected:
	void (*callback)(const char*);

public:
	LogCallback(void (*cbfunc)(const char*), unsigned int mask);
	virtual ~LogCallback();

	virtual void log(const char *msg, unsigned int type);
};

static vector<LogTarget*> log_list;

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

	LogFile *log;
	try {
		log = new LogFile(fname, mtypes);
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

	LogStream *log;
	try {
		log = new LogStream(fp, mtypes);
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

	LogCallback *log;
	try {
		log = new LogCallback(func, mtypes);
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

LogTarget::LogTarget(unsigned int mask)
{
	msg_mask = mask;
}

LogTarget::~LogTarget() {}

LogFile::LogFile(const char *fname, unsigned int mask)
	: LogTarget(mask)
{
	this->fname = new char[strlen(fname) + 1];
	strcpy(this->fname, fname);
	first_time = true;
}

LogFile::~LogFile()
{
	delete [] fname;
}

void LogFile::log(const char *msg, unsigned int type)
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

LogStream::LogStream(FILE *fp, unsigned int mask)
	: LogTarget(mask)
{
	this->fp = fp;
}

LogStream::~LogStream() {}

void LogStream::log(const char *msg, unsigned int type)
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

LogCallback::LogCallback(void (*cbfunc)(const char*), unsigned int mask)
	: LogTarget(mask)
{
	callback = cbfunc;
}

LogCallback::~LogCallback() {}

void LogCallback::log(const char *msg, unsigned int type)
{
	if(type & msg_mask) {
		callback(msg);
	}
}
