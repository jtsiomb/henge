#include <iostream>
#include <stdio.h>
#include "henge.h"
#include "opengl.h"
#include "sdr.h"
#include "errlog.h"
#include "datapath.h"

using namespace henge;

#if defined(unix) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define LOGFILE		"/tmp/henge.log"
#else
#define LOGFILE		"henge.log"
#endif

bool henge::init()
{
	set_log_stream(stderr, LOG_ALL);
	set_log_file(LOGFILE, LOG_ALL);
	remove(LOGFILE);

	if(!init_opengl()) {
		return false;
	}

	init_uniform_cache();
	init_textures();
	init_sdr();

	set_path(".");

	return true;
}

bool henge::init_no_gl()
{
	set_log_stream(stderr, LOG_WARNING | LOG_ERROR);
	set_log_file(LOGFILE, LOG_ALL);
	remove(LOGFILE);

	set_path(".");
	return true;
}

void henge::shutdown()
{
	destroy_textures();
	destroy_sdr();
}

static int vp[4] = {-1, -1, -1, -1};
void henge::set_viewport(int x, int y, int width, int height)
{
	vp[0] = x;
	vp[1] = y;
	vp[2] = width;
	vp[3] = height;

	glViewport(x, y, width, height);
}

void henge::get_viewport(int *x, int *y, int *width, int *height)
{
	if(vp[0] == -1) {
		glGetIntegerv(GL_VIEWPORT, vp);
	}
	if(x) *x = vp[0];
	if(y) *y = vp[1];
	if(width) *width = vp[2];
	if(height) *height = vp[3];
}
