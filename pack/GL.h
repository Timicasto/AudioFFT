#ifndef AUDIOFFT_GL_H
#define AUDIOFFT_GL_H

#include "glad/glad.h"
#include <cstdio>
#include <GLFW/glfw3.h>

class GL {
public:
	static GLFWwindow* initGLFW(int width, int height);

	static void initGLAD();

	static void initViewportDefault(GLFWwindow *pWwindow);
};


#endif //AUDIOFFT_GL_H
