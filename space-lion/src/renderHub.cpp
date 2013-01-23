#include "renderHub.h"


renderHub::renderHub(void)
{
}


renderHub::~renderHub(void)
{
}

bool renderHub::init()
{
	//	Initialize GLFW
	glfwInit();

	//	Open an OpenGL window
	glfwOpenWindow(300,300,0,0,0,0,0,0,GLFW_WINDOW);

	return true;
}

bool renderHub::addScene()
{
}

bool renderHub::deleteScene()
{
}

bool renderHub::setSceneParameters()
{
}

scene renderHub::getScene(const int index)
{
}

void renderHub::run()
{
	running = true;

	while(running)
	{
		activeScene->render();
	}
}