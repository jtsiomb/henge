#include "opengl.h"
#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "henge.h"

using namespace std;
using namespace henge;

void disp();
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void keyb_up(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);

Material mat;
Shader *sdr;
Texture *dif_tex, *norm_tex;

float cam_theta = 0, cam_phi = 0, cam_dolly = 5;
float cam_y = 0;

int main(int argc, char **argv)
{
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("henge :: normal map test");

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutKeyboardUpFunc(keyb_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_CULL_FACE);

	if(!henge::init()) {
		return 1;
	}

	if(!(sdr = get_shader("norm.v.glsl", "norm.p.glsl"))) {
		return 1;
	}
	sdr->set_uniform("tex_dif", 0);
	sdr->set_uniform("tex_norm", 1);

	if(!(dif_tex = get_texture("diffuse.png")) ||
			!(norm_tex = get_texture("normal.png"))) {
		return 1;
	}

	mat.set_color(Color(1.0, 0.9, 0.8, 1), MATTR_AMB_AND_DIF);
	mat.set_color(Color(1, 1, 1, 1), MATTR_SPECULAR);
	mat.set(60.0, MATTR_SHININESS);
	mat.set_shader(sdr);
	mat.set_texture(dif_tex, 0);
	mat.set_texture(norm_tex, 1);

	glutMainLoop();
	return 0;
}

float lpos[] = {-10, 0, 10, 1};

void disp()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, -cam_y, -cam_dolly);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	mat.bind();

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glVertexAttrib3fARB(SDR_ATTR_TANGENT, 1, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(-1, -1, 0);
	glTexCoord2f(1, 1); glVertex3f(1, -1, 0);
	glTexCoord2f(1, 0); glVertex3f(1, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, 1, 0);
	glEnd();

	set_shader(0);
	set_texture(0, 0);
	set_texture(0, 1);

	glutSwapBuffers();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)x / (float)y, 1.0, 1000.0);
}


static bool keydown[256];

void keyb(unsigned char key, int x, int y)
{
	keydown[(int)key] = true;

	switch(key) {
	case 27:
		exit(0);
	}
}

void keyb_up(unsigned char key, int x, int y)
{
	keydown[(int)key] = false;
}

int bnstate[16];

int prev_x = -1, prev_y;
void mouse(int bn, int state, int x, int y)
{
	bnstate[bn] = state == GLUT_DOWN ? 1 : 0;
	if(state == GLUT_DOWN) {
		if(bn == 3) {
			cam_dolly -= 0.5;
			glutPostRedisplay();
			if(cam_dolly < 0) cam_dolly = 0;
		} else if(bn == 4) {
			cam_dolly += 0.5;
			glutPostRedisplay();
		} else {
			prev_x = x;
			prev_y = y;
		}
	} else {
		prev_x = -1;
	}
}

void motion(int x, int y)
{
	if(keydown[(int)'l']) {
		lpos[0] += (x - prev_x) * 0.5;
		lpos[1] += (prev_y - y) * 0.5;
		glutPostRedisplay();
	} else {
		if(bnstate[0]) {
			cam_theta += (x - prev_x) * 0.5;
			cam_phi += (y - prev_y) * 0.5;

			if(cam_phi < -90) cam_phi = -90;
			if(cam_phi > 90) cam_phi = 90;

			glutPostRedisplay();
		}

		if(bnstate[1]) {
			cam_y += (prev_y - y) * 0.1;
			glutPostRedisplay();
		}

		if(bnstate[2]) {
			cam_dolly += (y - prev_y) * 0.1;
			glutPostRedisplay();
		}
	}

	prev_x = x;
	prev_y = y;
}
