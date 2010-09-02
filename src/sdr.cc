#include <map>
#include "opengl.h"
#include "sdr.h"
#include "unicache.h"
#include "errlog.h"
#include "datapath.h"

using namespace std;
using namespace henge;

static map<string, Shader*> sdrman;

static bool bind_program(unsigned int prog);

bool henge::init_sdr()
{
	return true;
}

void henge::destroy_sdr()
{
	map<string, Shader*>::iterator iter = sdrman.begin();
	while(iter != sdrman.end()) {
		delete iter++->second;
	}
	sdrman.clear();
}

bool henge::add_shader(const char *vname, const char *pname, Shader *sdr)
{
	if(!vname) vname = "";
	if(!pname) pname = "";

	char *key = (char*)alloca(strlen(vname) + strlen(pname) + 1);
	sprintf(key, "%s%s", vname, pname);

	try {
		sdrman[key] = sdr;
	}
	catch(...) {
		return false;
	}
	return true;
}

Shader *henge::get_shader(const char *vname, const char *pname)
{
	if(!vname && !pname) return 0;

	if(!vname) vname = "";
	if(!pname) pname = "";

	char *key = (char*)alloca(strlen(vname) + strlen(pname) + 1);
	sprintf(key, "%s%s", vname, pname);

	if(key[0] == 0) {
		return 0;
	}

	map<string, Shader*>::iterator iter = sdrman.find(key);
	if(iter != sdrman.end()) {
		return iter->second;
	}

	Shader *sdr;
	try {
		sdr = new Shader;
	}
	catch(...) {
		return 0;
	}

	if(*vname && !sdr->load_shader(vname, SDR_VERTEX)) {
		delete sdr;
		return 0;
	}
	if(*pname && !sdr->load_shader(pname, SDR_PIXEL)) {
		delete sdr;
		return 0;
	}
	if(!sdr->link()) {
		delete sdr;
		return 0;
	}
	add_shader(vname, pname, sdr);
	return sdr;
}

void henge::set_shader(const Shader *sdr)
{
	if(!caps.glsl) return;

	if(sdr) {
		sdr->bind();
	} else {
		glUseProgramObjectARB(0);
	}
}

Shader::Shader()
{
	prog = vsdr = psdr = 0;
	prog_valid = false;
}

Shader::~Shader()
{
	if(caps.glsl) {
		if(vsdr) glDeleteObjectARB(vsdr);
		if(psdr) glDeleteObjectARB(psdr);
		if(prog) glDeleteObjectARB(prog);
	}
}

void Shader::find_uniforms()
{
	uniforms.clear();

	int active_uniforms;
	glGetObjectParameterivARB(prog, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &active_uniforms);

	for(int i=0; i<active_uniforms; i++) {
		char name[1024];
		int size;
		unsigned int type;

		glGetActiveUniformARB(prog, i, sizeof name, 0, &size, &type, name);
		if(glGetError() != GL_NO_ERROR) {
			continue;
		}

		if(is_cached_uniform(name)) {
			info("found cached shader variable: %s\n", name);
			uniforms.push_back(name);
		}
	}
}

bool Shader::load_shader(const char *fname, SdrType type)
{
	char path[PATH_MAX];
	if(!find_file(fname, path)) {
		error("shader file not found: %s\n", fname);
		return false;
	}

	FILE *fp;

	if(!(fp = fopen(path, "rb"))) {
		error("failed to open shader file: %s\n", path);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	unsigned int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *src = (char*)alloca(filesize + 1);
	fread(src, 1, filesize, fp);
	src[filesize] = 0;

	return compile_shader(src, type, fname);
}

bool Shader::compile_shader(const char *src, SdrType type, const char *fname)
{
	if(!caps.glsl) {
		error("compile_shader failed, no GLSL support\n");
		return false;
	}

	if(!fname) fname = "<unknown>\n";
	info("compiling %s... ", fname);

	unsigned int sdr = glCreateShaderObjectARB(type);
	glShaderSourceARB(sdr, 1, &src, 0);
	glCompileShaderARB(sdr);

	int success, info_len;
	glGetObjectParameterivARB(sdr, GL_OBJECT_COMPILE_STATUS_ARB, &success);
	glGetObjectParameterivARB(sdr, GL_OBJECT_INFO_LOG_LENGTH_ARB, &info_len);

	char *info_str = 0;
	if(info_len) {
		info_str = new char[info_len + 1];
		glGetInfoLogARB(sdr, info_len, 0, info_str);
	}

	if(success) {
		info(info_str ? "done: %s\n" : "done\n", info_str);
	} else {
		error(info_str ? "failed: %s\n" : "failed\n", info_str);
		delete [] info_str;
		glDeleteObjectARB(sdr);
		return false;
	}
	delete [] info_str;

	unsigned int *sdr_var = type == SDR_VERTEX ? &vsdr : &psdr;
	if(*sdr_var) {
		glDeleteObjectARB(*sdr_var);
	}
	*sdr_var = sdr;

	prog_valid = false;
	return true;
}

bool Shader::link()
{
	if(!caps.glsl) {
		error("link failed, no GLSL support\n");
		return false;
	}

	if(!(vsdr || psdr)) {
		return false;
	}

	unsigned int prog = glCreateProgramObjectARB();
	glBindAttribLocationARB(prog, SDR_ATTR_TANGENT, "attr_tangent");

	if(vsdr) {
		glAttachObjectARB(prog, vsdr);
	}
	if(psdr) {
		glAttachObjectARB(prog, psdr);
	}
	glLinkProgramARB(prog);

	int linked, info_len;
	glGetObjectParameterivARB(prog, GL_OBJECT_LINK_STATUS_ARB, &linked);
	glGetObjectParameterivARB(prog, GL_OBJECT_INFO_LOG_LENGTH_ARB, &info_len);

	char *info_str = 0;
	if(info_len) {
		info_str = new char[info_len + 1];
		glGetInfoLogARB(prog, info_len, 0, info_str);
	}

	info("linking... ");
	if(linked) {
		info(info_str ? "done: %s\n" : "done\n", info_str);
	} else {
		error(info_str ? "failed: %s\n" : "failed\n", info_str);
		delete [] info_str;
		glDeleteObjectARB(prog);
		return false;
	}
	delete [] info_str;

	if(this->prog) {
		glDeleteObjectARB(this->prog);
	}
	this->prog = prog;

	prog_valid = true;

	// locate cached shader uniforms
	find_uniforms();

	return true;
}

// ugly but I'm not going to write the same bloody code over and over
#define BEGIN_UNIFORM_CODE \
	int loc, curr_prog; \
	if(!caps.glsl) { \
		return false; \
	} \
	if(!prog || !prog_valid) { \
		if(!((Shader*)this)->link()) { \
			return false; \
		} \
	} \
	glGetIntegerv(GL_CURRENT_PROGRAM, &curr_prog); \
	if((unsigned int)curr_prog != prog && !bind_program(prog)) { \
		return false; \
	} \
	if((loc = glGetUniformLocationARB(prog, name)) != -1)

#define END_UNIFORM_CODE \
	if((unsigned int)curr_prog != prog) { \
		bind_program(curr_prog); \
	} \
	return loc != -1


bool Shader::set_uniform(const char *name, int val) const
{
	BEGIN_UNIFORM_CODE {
		glUniform1iARB(loc, val);
	}
	END_UNIFORM_CODE;
}

bool Shader::set_uniform(const char *name, float val) const
{
	BEGIN_UNIFORM_CODE {
		glUniform1fARB(loc, val);
	}
	END_UNIFORM_CODE;
}

bool Shader::set_uniform(const char *name, double val) const
{
	BEGIN_UNIFORM_CODE {
		glUniform1fARB(loc, val);
	}
	END_UNIFORM_CODE;
}

bool Shader::set_uniform(const char *name, const Vector3 &vec) const
{
	BEGIN_UNIFORM_CODE {
		glUniform3fARB(loc, vec.x, vec.y, vec.z);
	}
	END_UNIFORM_CODE;
}

bool Shader::set_uniform(const char *name, const Vector4 &vec) const
{
	BEGIN_UNIFORM_CODE {
		glUniform4fARB(loc, vec.x, vec.y, vec.z, vec.w);
	}
	END_UNIFORM_CODE;
}

// XXX: do I need to transpose this ? probably...
bool Shader::set_uniform(const char *name, const Matrix4x4 &mat) const
{
	BEGIN_UNIFORM_CODE {
		glUniformMatrix4fvARB(loc, 1, 1, (float*)&mat);
	}
	END_UNIFORM_CODE;
}


bool Shader::bind() const
{
	if(!caps.glsl) {
		return false;
	}

	if(!prog || !prog_valid) {
		if(!((Shader*)this)->link()) {
			return false;
		}
	}

	for(size_t i=0; i<uniforms.size(); i++) {
		bind_cached_uniform(uniforms[i].c_str(), this);
	}

	return bind_program(prog);
}

static bool bind_program(unsigned int prog)
{
	glUseProgramObjectARB(prog);
	return glGetError() == GL_NO_ERROR;
}
