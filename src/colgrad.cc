#include <fstream>
#include <algorithm>
#include "colgrad.h"
#include "errlog.h"

using namespace henge;
using namespace std;

#define KEY_MAX		1024
#define KEY(t)		(int)((t) * (float)KEY_MAX)

static bool parse_sample(const char *start, float *t, color *c);

bool grad_sample::operator <(const grad_sample &rhs) const
{
	return key < rhs.key;
}

color_gradient::color_gradient()
{
	samples_sorted = true;
}

color_gradient::color_gradient(const color_gradient &grad)
{
	samples = grad.samples;
	samples_sorted = grad.samples_sorted;
	// don't copy the maps, let them be recreated when/if needed
}

color_gradient::~color_gradient()
{
	clear_maps();
}

void color_gradient::clear_maps()
{
	for(size_t i=0; i<maps.size(); i++) {
		delete maps[i];
	}
	ramp_cache.clear();
	rad_cache.clear();
}

color_gradient &color_gradient::operator =(const color_gradient &rhs)
{
	samples = rhs.samples;
	samples_sorted = rhs.samples_sorted;
	// don't copy the maps, let them be recreated when/if needed
	return *this;
}

void color_gradient::clear()
{
	samples.clear();
	clear_maps();
}

bool color_gradient::set_color(float t, const color &c)
{
	int key = KEY(t);

	if(key < 0) key = 0;
	if(key > KEY_MAX) key = KEY_MAX;

	grad_sample gs = {key, c};
	try {
		samples.push_back(gs);
	}
	catch(...) {
		return false;
	}

	samples_sorted = false;
	clear_maps();
	return true;
}

const color color_gradient::get_color(float t) const
{
	static const color dummy(1.0f, 1.0f, 1.0f, 1.0f);

	if(samples.empty()) {
		return dummy;
	}
	if(samples.size() == 1) {
		return samples[0].c;
	}

	if(!samples_sorted) {
		sort(samples.begin(), samples.end());
		samples_sorted = true;
	}

	int key = KEY(t);
	const grad_sample *start = 0, *end = 0;

	for(size_t i=0; i<samples.size(); i++) {
		if(key == samples[i].key) {
			return samples[i].c;
		}
		if(key < samples[i].key) {
			if(i == 0) {
				return samples[i].c;
			}
			start = &samples[i - 1];
			end = &samples[i];
			break;
		}
	}

	if(!start) {	// passed the end
		return samples[samples.size() - 1].c;
	}

	t = (float)(key - start->key) / (float)(end->key - start->key);
	return lerp(start->c, end->c, t);
}

bool color_gradient::load(const char *fname)
{
	ifstream file(fname);
	if(!file.is_open()) {
		error("failed to load color gradient: %s: open failed\n", fname);
		return false;
	}

	char sig[4];
	file.read(sig, 4);
	if(sig[0] != 'G' || sig[1] != 'R' || sig[2] != 'A' || sig[3] != 'D') {
		error("failed to load color gradient: %s: that's not a grad file\n", fname);
		return false;
	}

	clear();

	char line[256];
	for(;;) {
		file.getline(line, sizeof line);
		if(file.eof()) break;

		if(!*line || *line == '\n' || *line == '\r' || *line == '#') {
			continue;
		}

		float t;
		color c;
		if(!parse_sample(line, &t, &c)) {
			error("failed to load color gradient: %s: invalid or corrupted file\n", fname);
			return false;
		}
		set_color(t, c);
	}
	return true;
}

bool color_gradient::save(const char *fname) const
{
	ofstream file(fname);
	if(!file.is_open()) {
		error("failed to save color gradient: %s: open failed\n", fname);
		return false;
	}

	file << "GRAD\n";

	for(size_t i=0; i<samples.size(); i++) {
		file << ((float)samples[i].key / 1024.0f) << "\t";
		file << samples[i].c.x << " " << samples[i].c.y << " " << samples[i].c.z << "\n";
	}
	return true;
}

texture *color_gradient::ramp_map(int xsz, int ysz) const
{
	for(size_t i=0; i<ramp_cache.size(); i++) {
		if(ramp_cache[i]->get_width() == xsz && ramp_cache[i]->get_height() == ysz) {
			return ramp_cache[i];
		}
	}

	pixmap img;
	if(!img.set_pixels(xsz, ysz)) {
		return 0;
	}

	for(int x=0; x<xsz; x++) {
		const color c = get_color((float)x / (float)xsz);
		uint32_t pcol = pack_color(c);

		for(int y=0; y<ysz; y++) {
			((uint32_t*)img.pixels)[y * xsz + x] = pcol;
		}
	}

	texture_2d *tex = new texture_2d;
	tex->set_image(img);

	maps.push_back(tex);
	ramp_cache.push_back(tex);
	return tex;
}

texture *color_gradient::radial_map(int xsz, int ysz) const
{
	for(size_t i=0; i<rad_cache.size(); i++) {
		if(rad_cache[i]->get_width() == xsz && rad_cache[i]->get_height() == ysz) {
			return rad_cache[i];
		}
	}

	pixmap img;
	if(!img.set_pixels(xsz, ysz)) {
		return 0;
	}

	uint32_t *pix = (uint32_t*)img.pixels;
	for(int y=0; y<ysz; y++) {
		float fy = (float)(2 * y) / (float)ysz - 1.0f;
		for(int x=0; x<xsz; x++) {
			float fx = (float)(2 * x) / (float)xsz - 1.0f;

			float dist = sqrt(fx * fx + fy * fy);
			*pix++ = pack_color(get_color(dist));
		}
	}

	texture_2d *tex = new texture_2d;
	tex->set_image(img);

	maps.push_back(tex);
	rad_cache.push_back(tex);
	return tex;
}

static bool parse_sample(const char *start, float *t, color *c)
{
	float sdata[4];

	for(int i=0; i<4; i++) {
		char *end;
		sdata[i] = (float)strtod(start, &end);
		if(end == start) {
			return false;
		}
		start = end;
	}

	*t = sdata[0];
	c->x = sdata[1];
	c->y = sdata[2];
	c->z = sdata[3];
	c->w = 1.0f;

	return true;
}
