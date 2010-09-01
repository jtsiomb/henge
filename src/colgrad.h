#ifndef HENGE_COLOR_GRADIENT_H_
#define HENGE_COLOR_GRADIENT_H_

#include <vector>
#include "color.h"
#include "texture.h"

namespace henge {

struct grad_sample {
	int key; color c;

	bool operator <(const grad_sample &rhs) const;
};

class color_gradient {
private:
	// mutable due to lazy sorting in get_color.
	mutable std::vector<grad_sample> samples;
	mutable bool samples_sorted;

	// mutable due to caching in ramp_map/rad_map.
	mutable std::vector<texture*> maps;
	mutable std::vector<texture*> ramp_cache, rad_cache;

	void clear_maps();

public:
	color_gradient();
	color_gradient(const color_gradient &grad);
	~color_gradient();

	color_gradient &operator =(const color_gradient &rhs);

	void clear();
	bool set_color(float t, const color &c);
	const color get_color(float t) const;

	bool load(const char *fname);
	bool save(const char *fname) const;

	texture *ramp_map(int xsz = 128, int ysz = 1) const;
	texture *radial_map(int xsz, int ysz) const;
};

}

#endif	// HENGE_COLOR_GRADIENT_H_
