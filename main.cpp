#include <iostream>
#include "capture.h"
#include "GL.h"
#include "Engine.h"

FILE* file = nullptr;

int runFlag = 0;
void signalHandler(int signal) {
	runFlag = 1;
}

int main() {
	GLFWwindow* window = GL::initGLFW(1000, 1000);
	GL::initGLAD();
	GL::initViewportDefault(window);
	// initialize alsa
	Engine::loop(window);
	startCapture("default");
	glfwTerminate();
	return 0;
}
