#define _CRT_SECURE_NO_WARNINGS
#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION

#include <glm/glm.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "opengl_context.h"

#include "option.h"
#include "point.h"
#include "nbody.h"
#include "camera.h"

__declspec(align(0x10)) static point vertices_1[POINT_CNT];
__declspec(align(0x10)) static point vertices_2[POINT_CNT];

#define ANGEL_TO_RADIAN(x) (float)((x)*M_PI / 180.0f)

static void light()
{
	GLfloat light_specular[] = { 0.4, 0.4, 0.4, 1 };
	GLfloat light_diffuse[] = { 0.8,0.8,0.8, 1 };
	GLfloat light_ambient[] = { 0.6, 0.6, 0.6, 1 };
	GLfloat light_position[] = { 10, 5, 5, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
}

static void errorCallback(int error, const char *description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void resizeCallback(GLFWwindow *window, int width, int height)
{
	OpenGLContext::framebufferResizeCallback(window, width, height);
	auto ptr = static_cast<Camera *>(glfwGetWindowUserPointer(window));

	if (ptr) {
		ptr->updateProjectionMatrix(OpenGLContext::getAspectRatio());
	}
}

static void keyCallback(GLFWwindow *window, int key, int, int action, int)
{
	if (action == GLFW_REPEAT)
		return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void initOpenGL()
{
	OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
	GLFWwindow *window = OpenGLContext::getWindow();

	glfwSetWindowTitle(window, "NBodySim");
	glfwSetErrorCallback(errorCallback);
	glfwSetFramebufferSizeCallback(window, resizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

static void showVersion(void)
{
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion =
		glGetString(GL_SHADING_LANGUAGE_VERSION);

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	printf("GL Version (string)  : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version         : %s\n", glslVersion);
}

static void initPoints(void)
{
	for (int i = 0; i < POINT_CNT; ++i) {
#ifdef SEED
		unsigned int seed = SEED;
#else
#	ifdef _DEBUG
		unsigned int seed = 5566888 - 100 * i;
#	else
		unsigned int seed = time(NULL) - 100 * i;
#	endif
#endif

		vertices_1[i] = point(seed);
		vertices_2[i] = vertices_1[i];
	}
}

static void banner(void)
{
	printf("======== NBODYSIM ========\n");
	printf("1: Serial\n");
	printf("2: Serial SIMD\n");
	printf("3: Serial SIMD 2\n");
	printf("4: Parallel\n");
	printf("5: Parallel SIMD\n");
	printf("6: Parallel SIMD 2\n");
	printf("7: CUDA\n");
	printf("> ");
}

void nextdrawCircle(glm::quat rota, glm::vec3 *rotated, glm::vec3 *beforerotate)
{
	int i = 0;

	for (float angle = 0; angle < ANGEL_TO_RADIAN(360); angle += ANGEL_TO_RADIAN(360) / 15) {
		beforerotate[i].x = sin(angle);
		beforerotate[i].y = cos(angle);

		glm::vec3 rotate = rota * glm::vec3(beforerotate[i].x, beforerotate[i].y, 0);

		rotated[i].x = rotate.x;
		rotated[i].y = rotate.y;
		rotated[i].z = rotate.z;

		i++;
	}

	rotated[15].x = rotated[0].x;
	rotated[15].y = rotated[0].y;
	rotated[15].z = rotated[0].z;

	beforerotate[15].x = beforerotate[0].x;
	beforerotate[15].y = beforerotate[0].y;
}

void drawCircle(float px, float py, float pz, float R, glm::vec3 *rotate, glm::vec3 *beforerotate, glm::quat rota)
{
	int i = 0;

	glBegin(GL_TRIANGLE_FAN);

	glNormal3f(0.0f, 0.0f, 1);
	glVertex3f(px, py, pz);

	for (i = 15; i >= 0; i--) {
		glNormal3f(beforerotate[i].x, beforerotate[i].y, 0);
		glVertex3f(px + rotate[i].x * R, py + rotate[i].y * R, pz + rotate[i].z * R);
	}

	glEnd();
}

int main(void)
{
	GLFWwindow *window;
	double prevtime;
	point *points1 = vertices_1, *points2 = vertices_2;
	unsigned long long int estimate_round = 0;
	double estimate_time = 0, estimate_prevtime;
	nBodyFunc calculateNBody;
	double avgdt = 0.0;
	int choice;

	initOpenGL();

	window = OpenGLContext::getWindow();

	// Init Camera helper
	Camera camera(glm::vec3(0, 0, 5));
	camera.initialize(OpenGLContext::getAspectRatio());

	// Store camera as glfw global variable for callbasks use
	glfwSetWindowUserPointer(window, &camera);

	showVersion();

	initPoints();

	banner();
	scanf("%d", &choice);
	switch (choice) {
	case 1:
		calculateNBody = nBodyCalculateSerial;
		break;
	case 2:
		calculateNBody = nBodyCalculateSerialSIMD;
		break;
	case 3:
		calculateNBody = nBodyCalculateSerialSIMD256;
		break;
	case 4:
		calculateNBody = nBodyCalculateParallel;
		break;
	case 5:
		calculateNBody = nBodyCalculateParallelSIMD;
		break;
	case 6:
		calculateNBody = nBodyCalculateParallelSIMD256;
		break;
	case 7:
		calculateNBody = nBodyCalculateCUDA;
		break;
	default:
		calculateNBody = nBodyCalculateSerial;
	}

	prevtime = glfwGetTime();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

#ifndef DISABLE_LIGHT
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHT0);
	light();
#endif

	while (!glfwWindowShouldClose(window)) {
		double dt; // delta time
		double currtime;
		point *tmp;

		// Polling events.
		glfwPollEvents();

		// Update camera position and view
		camera.move(window);

		// GL_XXX_BIT can simply "OR" together to use.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(camera.getProjectionMatrix());

		// ModelView Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(camera.getViewMatrix());

#ifndef DISABLE_LIGHT
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
#endif

		currtime = (float)glfwGetTime();
		dt = currtime - prevtime;
		prevtime = currtime;
		avgdt += dt;

		estimate_round += 1;
		estimate_prevtime = glfwGetTime();

		calculateNBody(points1, points2, dt * DELTA_TIME_MUL);

		estimate_time += glfwGetTime() - estimate_prevtime;

		tmp = points1;
		points1 = points2;
		points2 = tmp;

		glScalef(0.0001f, 0.0001f, 0.0001f);

		// set drawcircle  
		glm::quat rota = camera.rotation;
		glm::vec3 rotated[16];
		glm::vec3 beforerotate[16];
		nextdrawCircle(rota, rotated, beforerotate);

		for (int i = 0; i < POINT_CNT; i++) {
			glColor3f(points1[i]._r, points1[i]._g, points1[i]._b);
			drawCircle(points1[i]._x, points1[i]._y, points1[i]._z, points1[i]._mass, rotated, beforerotate, rota);
		}

		// Update FPS
		if (dt != 0 && estimate_round % 10 == 0) {
			avgdt /= 10;
			std::string title = "N = " + std::to_string((int)POINT_CNT) + ", FPS = " + std::to_string((int)(1 / avgdt));
			avgdt = 0;
			glfwSetWindowTitle(window, title.c_str());
		}

		glfwSwapBuffers(window);

#ifdef ESTIMATION
		if (estimate_round == 100) {
			break;
		}
#endif
	}

	printf("Total round: %lld\n", estimate_round);
	printf("Avg calculating time: %f ms\n", estimate_time * 1000 / estimate_round);

	exit(EXIT_SUCCESS);
}