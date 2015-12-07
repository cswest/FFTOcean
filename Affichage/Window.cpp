#ifndef FENETREHPP
#define FENETREHPP

#include "Camera.hpp"
#include "Window.hpp"

#ifdef __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
#endif
#ifdef __linux__
    #include <GL/glut.h>
#else
    #include <GLUT/glut.h>
#endif

#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>

namespace Window {
	
	/* For every GLUT project */
	Camera			camera(-100, 100, -100, M_PI/4, 4*M_PI/7, 0.01, 0.1, WIDTH, HEIGHT);
	int				frames(-1);
	int				fps;
    int				fps_goal;  // expected FPS
    std::string		fps_str;
	time_t			sleep_avant(0);
	int				t;
	struct timespec tim1, tim2;

	/* For FFT-Ocean */
	int		nxOcean;
	int		nyOcean;
	double *vertexOceanX; // to call glVertexPointer
    double *vertexOceanY; // to call glVertexPointer
    
    /* To duplicate the waves */
    int	height = 1;
    int width = 1;

    /* Main draw function */
	void draw() {
		if(glutGet(GLUT_ELAPSED_TIME) - t >= 1000) fps_action();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		camera.translation();
		gluLookAt(camera.getX(), camera.getY(), camera.getZ(), camera.getSightX(), camera.getSightY(), camera.getSightZ(), 0, 1, 0);
		draw_ocean();
		draw_fps();
		glutSwapBuffers();
		glutPostRedisplay();
		nanosleep(&tim1, &tim2);
		frames++;
	}
	
    /* Draws the FPS in the top right corner */
	void draw_fps() {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 640, 0, 480);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRasterPos2f(550, 460);
		char       buf[15] = "FPS : ";
		const char *p(buf);
		strcat(buf, fps_str.c_str());
		do glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p); while(*(++p));
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	
    /* Draws the ocean */
	void draw_ocean() {
        ocean->mainComputation();
		glColor3ub(82, 184, 255);
		for(int offsetX = 0 ; offsetX <= width ; offsetX++) {
			for(int offsetY = 0 ; offsetY <= height ; offsetY++) {
				for(int x = 0 ; x < nxOcean ; x++) {
					ocean->glVertexArrayY(x, vertexOceanY, offsetX, offsetY);
					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(3, GL_DOUBLE, 0, vertexOceanY);
					glDrawArrays(GL_LINE_STRIP, 0, nyOcean+1);
					glDisableClientState(GL_VERTEX_ARRAY);
				}
				for(int y = 0 ; y < nyOcean ; y++) {
					ocean->glVertexArrayX(y, vertexOceanX, offsetX, offsetY);
					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(3, GL_DOUBLE, 0, vertexOceanX);
					glDrawArrays(GL_LINE_STRIP, 0, nxOcean+1);
					glDisableClientState(GL_VERTEX_ARRAY);
				}
			}
		}
		glColor3ub(0, 0, 0);
	}
	
    /* Maintains a stable FPS */
	void fps_action() {
		t      = glutGet(GLUT_ELAPSED_TIME);
		fps    = frames;
		frames = 0;
		std::ostringstream oss;
		oss << fps;
		fps_str = oss.str();
		sleep_avant  = tim1.tv_nsec;
		tim1.tv_nsec = (int)(((double)(1.0/fps_goal) - (double)(1.0/fps))*pow(10, 9) + sleep_avant) % 1000000000;
	}
    
    /* OpenGL init function */
	void init(int width, int height, std::string titre, int argc, char **argv) {
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
		glutInitWindowSize(width, height);
		mainwindow = glutCreateWindow(titre.c_str());
		glEnable(GL_MULTISAMPLE);
	}
	
    /* Keyboard function */
	void keyboard(unsigned char key, int x, int y) {
		camera.setKeyboard(key, true);
	}
	
    /* Keyboard function */
	void keyboardUp(unsigned char key, int x, int y) {
		camera.setKeyboard(key, false);
	}

    /* Starts the rendering */
	void launch() {
#ifdef __APPLE__
		CGSetLocalEventsSuppressionInterval(0.0);
#endif
		tim1.tv_sec  = 0;
		tim1.tv_nsec = 0;
		t = glutGet(GLUT_ELAPSED_TIME);
		nxOcean = ocean->getNx();
		nyOcean = ocean->getNy();
		vertexOceanX = new double[3*(nxOcean+1)];
		vertexOceanY = new double[3*(nyOcean+1)];
		glClearColor(1, 1, 1, 1);
		glutReshapeFunc(reshape);
		glutDisplayFunc(draw);
		glutPassiveMotionFunc(mouseMove);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardUp);
		glutWarpPointer(WIDTH/2, HEIGHT/2+40);     // +40 for mac
		glutMainLoop();
	}
	
    /* Mouse function */
	void mouseMove(int x, int y) {
		camera.rotation(x, y);
		if(x >= WIDTH || x <= 0 || y >= HEIGHT || y <= 0) {
			glutWarpPointer(WIDTH/2, HEIGHT/2);
			camera.setMouse(WIDTH/2, HEIGHT/2);
		}
	}
	
    /* Memory freeing */
	void quit() {
		delete[] vertexOceanX;
		delete[] vertexOceanY;
	}

    /* Reshape function */
	void reshape(int width, int height) {
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, float(width)/float(height), 1, 100*ocean->getNy());    // (near, .., .., far)
	}
    
    /* fps accessor */
	void setFPS(int f) {
		fps_goal = f;
	}
	
}

#endif