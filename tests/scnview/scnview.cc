#include <float.h>
#include <assert.h>
#include "opengl.h"
#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "henge.h"

using namespace henge;

void disp();
void env_render(void);
void object_render(const RObject *obj, unsigned int msec, void *cls);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void keyb_up(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);

Material mat;
Texture *cube_tex;
Scene *scn;

float cam_theta = 0, cam_phi = 0, cam_dist = 50;
float cam_y = 0;

int main(int argc, char **argv)
{
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("henge :: scene viewer");

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

	scn = new Scene;

	int num_loaded = 0;
	for(int i=1; i<argc; i++) {
		if(!scn->load(argv[i])) {
			fprintf(stderr, "failed to load scene: %s\n", argv[i]);
		} else {
			num_loaded++;
		}
	}

	if(!num_loaded) {
		return 1;
	}

	float scnsize = (scn->get_bbox()->max - scn->get_bbox()->min).length();
	if(scnsize < FLT_MAX) {
		cam_dist = scnsize / tan(M_PI / 4.0);
	}

	std::cout << "scene bounds:\n";
	std::cout << "  " << scn->get_bbox()->min << "\n";
	std::cout << "  " << scn->get_bbox()->max << "\n";
	std::cout << "cam dist: " << cam_dist << std::endl;

	glutMainLoop();
	return 0;
}

void disp()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float lpos[] = {-1, 1, 3, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	glTranslatef(0, -cam_y, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	scn->render();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
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
			cam_dist -= 0.5;
			glutPostRedisplay();
			if(cam_dist < 0) cam_dist = 0;
		} else if(bn == 4) {
			cam_dist += 0.5;
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
		cam_dist += (y - prev_y) * 0.1;
		glutPostRedisplay();
	}

	prev_x = x;
	prev_y = y;
}
