#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "datapath.h"
#include "errlog.h"

#ifdef MSC_VER_
#include <malloc.h>
#else
#include <alloca.h>
#endif

using namespace henge;
using namespace std;

static vector<string> path_dirs;

bool henge::set_path(const char *path)
{
	path_dirs.clear();
	return add_path(path);
}

bool henge::add_path(const char *path)
{
	char *mypath = (char*)alloca(strlen(path) + 1);
	strcpy(mypath, path);

	char *tok = strtok(mypath, ":");
	while(tok) {
		try {
			path_dirs.push_back(string(tok));
			if(tok[strlen(tok) - 1] != '/') {
				path_dirs[path_dirs.size() - 1] += "/";
			}
		}
		catch(...) {
			return false;
		}
		tok = strtok(0, ":");
	}
	return true;
}

const char *henge::get_path()
{
	static string path;

	path.clear();
	for(size_t i=0; i<path_dirs.size(); i++) {
		if(i > 0) {
			path += ":";
		}
		path += path_dirs[i];
	}
	return path.c_str();
}

bool henge::find_file(const char *fname, char *pathname, int pnsz)
{
	while(*fname && isspace(*fname)) {
		fname++;
	}

	if(*fname == '/') {
		strncpy(pathname, fname, pnsz);
		return access(fname, F_OK) == 0;
	}

	info("looking for: %s: ", fname);
	for(size_t i=0; i<path_dirs.size(); i++) {
		string pname = path_dirs[i] + fname;
		info("%s ... ", pname.c_str());
		if(access(pname.c_str(), F_OK) == 0) {
			if(pathname) {
				strncpy(pathname, pname.c_str(), pnsz);
			}
			info("found\n");
			return true;
		}
	}
	info("not found\n");
	return false;
}
