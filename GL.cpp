#include "GL.h"

GLFWwindow* GL::initGLFW(int width, int height) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 16);

	GLFWwindow* window = glfwCreateWindow(width, height, "AudioFFT", nullptr, nullptr);
	if (window == nullptr) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);
	return window;
}

void GL::initGLAD() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		glfwTerminate();
	}
}

void fboCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void GL::initViewportDefault(GLFWwindow *pWwindow) {
	int width, height;
	glfwGetWindowSize(pWwindow, &width, &height);
	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(pWwindow, fboCallback);
}
