#define SKY_CC_IMPL

#include <algorithm>
#include "sky.h"
#include "sky_sdr.h"
#include "ggen.h"
#include "psys.h"

using namespace henge;

static Texture *create_perm_texture();
static Texture *perm_tex;

static bool layer_cmp(const SkyLayer *a, const SkyLayer *b)
{
	return a->get_radius() > b->get_radius();
}

SkyDome::SkyDome()
{
	sun_tex = moon_tex = 0;

	sun_pos = SphVector(0, QUARTER_PI, 600);
	sun_size = 10.0f;

	moon_pos = SphVector(PI, QUARTER_PI, 800);
	moon_size = 10.0f;
}

SkyDome::~SkyDome()
{
	for(size_t i=0; i<layers.size(); i++) {
		delete layers[i];
	}
}

bool SkyDome::add_layer(SkyLayer *layer)
{
	try {
		layers.push_back(layer);
	}
	catch(...) {
		return false;
	}
	sort(layers.begin(), layers.end(), layer_cmp);
	return true;
}

void SkyDome::set_sun_tex(Texture *tex)
{
	sun_tex = tex;
}

void SkyDome::set_moon_tex(Texture *tex)
{
	moon_tex = tex;
}

void SkyDome::set_sun_pos(const SphVector &svec)
{
	sun_pos = svec;
}

void SkyDome::set_moon_pos(const SphVector &svec)
{
	moon_pos = svec;
}

void SkyDome::set_sun_size(float sz)
{
	sun_size = sz;
}

void SkyDome::set_moon_size(float sz)
{
	moon_size = sz;
}

void SkyDome::draw(unsigned int msec) const
{
	bool drawn_sun = false, drawn_moon = false;

	for(size_t i=0; i<layers.size(); i++) {
		if(!drawn_moon && moon_pos.r > layers[i]->get_radius()) {
			draw_moon();
			drawn_moon = true;
		}
		if(!drawn_sun && sun_pos.r > layers[i]->get_radius()) {
			draw_sun();
			drawn_sun = true;
		}
		if(layers[i]->is_enabled()) {
			layers[i]->draw(msec);
		}
	}

	if(!drawn_moon) {
		draw_moon();
	}
	if(!drawn_sun) {
		draw_sun();
	}
}

void SkyDome::draw_sun() const
{
	if(!sun_tex) return;

	glPushAttrib(GL_ENABLE_BIT);
	set_texture(sun_tex);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	draw_point(Vector3(sun_pos), Color(1, 1, 1, 1), sun_size);

	glPopAttrib();
}

void SkyDome::draw_moon() const
{
	if(!moon_tex) return;

	glPushAttrib(GL_ENABLE_BIT);
	set_texture(moon_tex);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	draw_point(Vector3(moon_pos), Color(1, 1, 1, 1), moon_size);

	glPopAttrib();
}

// -- SkyDome layers base class (abstract)
SkyLayer::SkyLayer(float rad)
{
	radius = rad;
	col = Color(1, 1, 1, 1);
	enabled = true;

	dome = new RObject;
	dome->get_material_ptr()->set_color(Color(1, 1, 1, 1));
	gen_sphere(dome->get_mesh(), 1.0, 40, 10, 1, 0.2, TC_MAP_XZ);
}

SkyLayer::~SkyLayer()
{
	delete dome;
	delete mat;
}

void SkyLayer::enable()
{
	enabled = true;
}

void SkyLayer::disable()
{
	enabled = false;
}

bool SkyLayer::is_enabled() const
{
	return enabled;
}

void SkyLayer::set_radius(float rad)
{
	radius = rad;
}

float SkyLayer::get_radius() const
{
	return radius;
}

void SkyLayer::set_color(const henge::Color &col)
{
	this->col = col;
}

const henge::Color &SkyLayer::get_color() const
{
	return col;
}

void SkyLayer::draw(unsigned int msec) const
{
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDepthMask(0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0, -radius * cos(0.2 * M_PI), 0);
	glScalef(radius, -radius, radius);

	glColor4f(col.x, col.y, col.z, col.w);
	dome->render(msec);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

// -- gradient SkyDome layer
SkyLayerGrad::SkyLayerGrad(float rad)
	: SkyLayer(rad)
{
	def_grad = true;
	grad.set_color(0, Color(0.55f, 0.55f, 0.6f, 1));
	grad.set_color(0.2f, Color(0.4f, 0.5f, 0.7f, 1));
	grad.set_color(1.0f, Color(0.2f, 0.2f, 0.9f, 1));

	gen_sphere(dome->get_mesh(), 1.0, 40, 10, 1, 0.2, TC_MAP_XY);
}

SkyLayerGrad::SkyLayerGrad(const ColorGradient &grad)
{
	this->grad = grad;
}

SkyLayerGrad::~SkyLayerGrad() {}

bool SkyLayerGrad::load_grad(const char *fname)
{
	return grad.load(fname);
}

bool SkyLayerGrad::save_grad(const char *fname)
{
	return grad.save(fname);
}

void SkyLayerGrad::set_grad_color(float t, const Color &col)
{
	if(def_grad) {
		grad.clear();
		def_grad = false;
	}
	grad.set_color(t, col);
}

void SkyLayerGrad::draw(unsigned int msec) const
{
	Texture *tex = grad.ramp_map();
	tex->set_wrap(GL_CLAMP_TO_EDGE);
	dome->get_material_ptr()->set_texture(tex);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.5, 0.5, 0);
	glRotatef(90, 0, 0, 1);
	glTranslatef(-0.5, -0.5, 0);
	glScalef(5, 5, 1);

	SkyLayer::draw(msec);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}

// -- clouds SkyDome layer
SkyLayerClouds::SkyLayerClouds(float rad)
	: SkyLayer(rad)
{
	scale = Vector2(0.6f, 0.6f);
	velocity = Vector2(0, 0);
}

SkyLayerClouds::~SkyLayerClouds() {}

void SkyLayerClouds::set_scale(float scale)
{
	this->scale.x = this->scale.y = this->scale.z = scale;
}

void SkyLayerClouds::set_scale(const Vector3 &scale)
{
	this->scale = scale;
}

void SkyLayerClouds::set_velocity(const Vector2 &vel)
{
	velocity = vel;
}

// -- clouds on a texture SkyDome layer
SkyLayerCloudsTex::SkyLayerCloudsTex(float rad, Texture *tex)
	: SkyLayerClouds(rad)
{
	dome->get_material_ptr()->set_texture(tex);
}

SkyLayerCloudsTex::~SkyLayerCloudsTex() {}

void SkyLayerCloudsTex::set_texture(Texture *tex)
{
	dome->get_material_ptr()->set_texture(tex);
}

void SkyLayerCloudsTex::draw(unsigned int msec) const
{
	float sec = (float)msec / 1000.0f;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(sec * velocity.x, sec * velocity.y, 0);
	glScalef(scale.x, scale.y, 1);

	SkyLayer::draw(msec);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}

// procedural clouds SkyDome layer
SkyLayerCloudsSdr::SkyLayerCloudsSdr(float rad, Shader *sdr)
	: SkyLayerClouds(rad)
{
	coverage = 0.6f;
	sharpness = 0.7f;
	anim_speed = 0;

	if(!perm_tex) {
		perm_tex = create_perm_texture();
	}
	dome->get_material_ptr()->set_texture(perm_tex);

	if(!sdr) {	// load default shaders
		Shader *s = new Shader;
		if(!s->compile_shader(def_clouds_vs, SDR_VERTEX, "default clouds vertex shader") ||
				!s->compile_shader(def_clouds_ps, SDR_PIXEL, "default clouds pixel shader")) {
			delete s;
		} else {
			set_shader(s);
		}
	} else {
		set_shader(sdr);
	}
}

SkyLayerCloudsSdr::~SkyLayerCloudsSdr() {}

void SkyLayerCloudsSdr::set_shader(Shader *sdr)
{
	dome->get_material_ptr()->set_shader(sdr);
}

void SkyLayerCloudsSdr::set_anim_speed(float s)
{
	anim_speed = s;
}

void SkyLayerCloudsSdr::set_coverage(float cov)
{
	coverage = cov;
}

void SkyLayerCloudsSdr::set_sharpness(float shrp)
{
	sharpness = shrp;
}

void SkyLayerCloudsSdr::draw(unsigned int msec) const
{
	float sec = (float)msec / 1000.0f;

	Shader *sdr = dome->get_material_ptr()->get_shader();
	if(sdr) {
		sdr->set_uniform("pixw", 1.0 / (float)perm_tex->get_width());
		sdr->set_uniform("halfpixw", 0.5 / (float)perm_tex->get_width());
		sdr->set_uniform("cloud_cover", coverage);
		sdr->set_uniform("cloud_sharpness", sharpness);
	}

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(sec * velocity.x, sec * velocity.y, sec * anim_speed);
	glScalef(scale.x, scale.y, scale.z);

	SkyLayer::draw(msec);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	if(sdr) {
		::set_shader(0);
	}
}

static int perm[256]= {
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

static int grad3[16][3] = {
	{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
	{1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
	{1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
	{1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}
};

static Texture *create_perm_texture()
{
	henge::Pixmap img;
	img.set_pixels(256, 256);

	unsigned char *pixels = (unsigned char*)img.pixels;

	for(int i=0; i<256; i++) {
		for(int j=0; j<256; j++) {
			int offset = (i * 256 + j) * 4;
			char value = perm[(j+perm[i]) & 0xff];
			pixels[offset+2] = grad3[value & 0xf][0] * 64 + 64;	/* Gradient x */
			pixels[offset+1] = grad3[value & 0xf][1] * 64 + 64;	/* Gradient y */
			pixels[offset+0] = grad3[value & 0xf][2] * 64 + 64;	/* Gradient z */
			pixels[offset+3] = value;							/* Permuted index */
		}
	}

	Texture2D *tex = new Texture2D;
	tex->set_image(img);
	tex->set_wrap(GL_REPEAT);
	tex->set_filter(GL_NEAREST, GL_NEAREST);

	add_texture("noise_perm_tex", tex);
	return tex;
}
