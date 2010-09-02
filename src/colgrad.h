#ifndef HENGE_COLOR_GRADIENT_H_
#define HENGE_COLOR_GRADIENT_H_

#include <vector>
#include "color.h"
#include "texture.h"

namespace henge {

struct GradSample {
	int key;
	Color c;

	bool operator <(const GradSample &rhs) const;
};

class ColorGradient {
private:
	// mutable due to lazy sorting in get_color.
	mutable std::vector<GradSample> samples;
	mutable bool samples_sorted;

	// mutable due to caching in ramp_map/rad_map.
	mutable std::vector<Texture*> maps;
	mutable std::vector<Texture*> ramp_cache, rad_cache;

	void clear_maps();

public:
	ColorGradient();
	ColorGradient(const ColorGradient &grad);
	~ColorGradient();

	ColorGradient &operator =(const ColorGradient &rhs);

	void clear();
	bool set_color(float t, const Color &c);
	const Color get_color(float t) const;

	bool load(const char *fname);
	bool save(const char *fname) const;

	Texture *ramp_map(int xsz = 128, int ysz = 1) const;
	Texture *radial_map(int xsz, int ysz) const;
};

}

#endif	// HENGE_COLOR_GRADIENT_H_
