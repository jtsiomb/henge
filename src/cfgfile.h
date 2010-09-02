#ifndef HENGE_CFGFILE_H_
#define HENGE_CFGFILE_H_

#include <map>
#include <string>
#include "vmath.h"

namespace henge {

enum ConfigValueType {
	CVAL_INVALID,
	CVAL_SCALAR,
	CVAL_VECTOR,
	CVAL_STRING,
	CVAL_BOOL
};

struct ConfigValue {
	ConfigValueType type;

	float val;
	Vector4 vec;
	std::string str;
	bool bval;

	ConfigValue();
};

class ConfigFile {
private:
	std::map<std::string, ConfigValue> options;

public:
	bool read(const char *fname);

	void setopt(const char *name, float val);
	void setopt(const char *name, const Vector4 &vec);
	void setopt(const char *name, const char *str);
	void setopt(const char *name, bool val);

	bool getopt(const char *name, float *val) const;
	bool getopt(const char *name, Vector4 *vec) const;
	bool getopt(const char *name, char **str) const;
	bool getopt(const char *name, std::string *str) const;
	bool getopt(const char *name, bool *val) const;

	// convenience wrapper for getopt(..., float)
	bool getopt_int(const char *name, int *val) const;
};

}	// namespace henge

#endif	// HENGE_CFGFILE_H_
