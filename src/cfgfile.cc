#include <fstream>
#include <string.h>
#include <ctype.h>
#include "cfgfile.h"
#include "errlog.h"

using namespace std;
using namespace henge;

static bool is_number(const char *str);
static bool is_bool(const char *str);
static bool parse_vector(const char *str, Vector4 *vec);


ConfigValue::ConfigValue()
{
	type = CVAL_INVALID;
	val = 0.0f;
	bval = false;
}


bool ConfigFile::read(const char *fname)
{
	ifstream file(fname);
	if(!file.is_open()) {
		error("failed to open config file: %s\n", fname);
		return false;
	}

	char buf[512];
	int line = 0;
	for(;;) {
		char *lhs, *rhs, *ptr;
		line++;

		file.getline(buf, sizeof buf);
		if(file.eof()) {
			break;
		}

		if((ptr = strchr(buf, '#'))) {
			*ptr = 0;
		}
		for(ptr=buf; *ptr && isspace(*ptr); ptr++);

		if(!*ptr || *ptr == '\n' || *ptr == '\r') {
			continue;
		}

		lhs = ptr;	// left hand side starts from the first non-white char
		while(*ptr && !isspace(*ptr) && !(*ptr == ':' || *ptr == '=')) {
			*ptr++;
		}
		if(!*ptr) {
			warning("malformed config file: %s line: %d\n", fname, line);
			continue;
		}
		*ptr++ = 0;

		while(*ptr && (*ptr == ':' || *ptr == '=' || isspace(*ptr))) {
			ptr++;
		}
		if(!*ptr) {
			warning("malformed config file: %s line: %d\n", fname, line);
			continue;
		}
		rhs = ptr;	// right hand side...

		ConfigValue v;
		v.str = rhs;

		if(parse_vector(rhs, &v.vec)) {
			v.type = CVAL_VECTOR;
		} else if(is_number(rhs)) {
			v.type = CVAL_SCALAR;
			v.val = v.vec.x;
		} else if(is_bool(rhs)) {
			v.type = CVAL_BOOL;
			v.bval = (v.str == "true" || v.str == "yes") ? true : false;
		} else {
			v.type = CVAL_STRING;
		}
		options[lhs] = v;
	}

	return true;
}


void ConfigFile::setopt(const char *name, float val)
{
	options[name].type = CVAL_SCALAR;
	options[name].val = val;
}

void ConfigFile::setopt(const char *name, const Vector4 &vec)
{
	options[name].type = CVAL_VECTOR;
	options[name].vec = vec;
}

void ConfigFile::setopt(const char *name, const char *str)
{
	options[name].type = CVAL_STRING;
	options[name].str = str;
}

void ConfigFile::setopt(const char *name, bool val)
{
	options[name].type = CVAL_BOOL;
	options[name].bval = val;
}


bool ConfigFile::getopt(const char *name, float *val) const
{
	map<string, ConfigValue>::const_iterator iter = options.find(name);
	if(iter == options.end()) {
		return false;
	}

	ConfigValueType type = iter->second.type;

	switch(type) {
	case CVAL_SCALAR:
		*val = iter->second.val;
		break;

	case CVAL_VECTOR:
		*val = iter->second.vec.x;
		break;

	case CVAL_BOOL:
		*val = iter->second.bval ? 1.0 : 0.0;
		break;

	default:
		return false;
	}
	return true;
}

bool ConfigFile::getopt(const char *name, Vector4 *vec) const
{
	map<string, ConfigValue>::const_iterator iter = options.find(name);
	if(iter == options.end()) {
		return false;
	}

	ConfigValueType type = iter->second.type;

	switch(type) {
	case CVAL_SCALAR:
		vec->x = vec->y = vec->z = vec->w = iter->second.val;
		break;

	case CVAL_VECTOR:
		*vec = iter->second.vec;
		break;

	case CVAL_BOOL:
		vec->x = vec->y = vec->z = vec->w = iter->second.bval ? 1.0 : 0.0;
		break;

	default:
		return false;
	}
	return true;
}

bool ConfigFile::getopt(const char *name, char **str) const
{
	map<string, ConfigValue>::const_iterator iter = options.find(name);
	if(iter == options.end()) {
		return false;
	}

	*str = (char*)iter->second.str.c_str();
	return true;
}

bool ConfigFile::getopt(const char *name, std::string *str) const
{
	map<string, ConfigValue>::const_iterator iter = options.find(name);
	if(iter == options.end()) {
		return false;
	}

	*str = iter->second.str.c_str();
	return true;

}

bool ConfigFile::getopt(const char *name, bool *val) const
{
	map<string, ConfigValue>::const_iterator iter = options.find(name);
	if(iter == options.end()) {
		return false;
	}

	ConfigValueType type = iter->second.type;

	switch(type) {
	case CVAL_SCALAR:
		*val = iter->second.val > 1e-4 || iter->second.val < -1e-4;
		break;

	case CVAL_VECTOR:
		*val = iter->second.vec.x > 1e-4 || iter->second.vec.x < -1e-4;
		break;

	case CVAL_BOOL:
		*val = iter->second.bval;
		break;

	default:
		return false;
	}
	return true;
}

bool ConfigFile::getopt_int(const char *name, int *val) const
{
	float tmp;
	if(getopt(name, &tmp)) {
		*val = (int)tmp;
		return true;
	}
	return false;
}

static bool is_number(const char *str)
{
	char *tmp;
	strtod(str, &tmp);
	return tmp != str;
}

static bool is_bool(const char *str)
{
	static const char *bool_str[] = {"true", "false", "yes", "no", 0};

	for(int i=0; bool_str[i]; i++) {
		if(strcmp(str, bool_str[i]) == 0) {
			return true;
		}
	}
	return false;
}

static bool parse_vector(const char *str, Vector4 *vec)
{
	int i;

	for(i=0; i<4; i++) {
		char *tmp;
		(*vec)[i] = strtod(str, &tmp);
		if(tmp == str) {
			break;
		}
		str = tmp;

		while(*str && (*str == ',' || isspace(*str))) {
			str++;
		}
	}

	if(i < 2) vec->y = vec->x;
	if(i < 3) vec->z = vec->y;
	if(i < 4) vec->w = 1.0;

	return i > 0;
}
