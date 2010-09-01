#include "post.h"

using namespace henge;

void henge::overlay(const texture *tex, const color &col, const shader *sdr)
{
	overlay(Vector2(0, 0), Vector2(1, 1), col, tex, sdr, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void henge::overlay(const Vector2 &pos, const Vector2 &sz, const color &col,
		const texture *tex, const shader *sdr, unsigned int src_blend, unsigned int dst_blend)
{
	glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(src_blend, dst_blend);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(2.0f * pos.x - 1, 2.0f * pos.y - 1, 0);
	glScalef(sz.x, sz.y, 1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	if(tex) tex->bind();
	if(sdr) sdr->bind();

	glBegin(GL_QUADS);
	glColor4f(col.x, col.y, col.z, col.w);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(1, 0); glVertex2f(2, 0);
	glTexCoord2f(1, 1); glVertex2f(2, 2);
	glTexCoord2f(0, 1); glVertex2f(0, 2);
	glEnd();

	if(tex) set_texture(0);
	if(sdr) set_shader(0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}
