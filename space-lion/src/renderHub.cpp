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

	//	Open a glfw window
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

	if(!glfwOpenWindow(300,300,0,0,0,0,0,0,GLFW_WINDOW))
	{
		return false;
	}

	//	Initialize glew
	GLenum error = glewInit();
	if( GLEW_OK != error)
	{
		std::cout<<"-----\n"
				<<"The time is out of joint - O cursed spite,\n"
				<<"That ever I was born to set it right!\n"
				<<"-----\n"
				<<"Error: "<<glewGetErrorString(error);
		return false;
	}

	return true;
}

bool renderHub::addScene()
{
	sceneList.push_back(scene());

	return true;
}

bool renderHub::deleteScene()
{
}

bool renderHub::setSceneParameters()
{
}

scene* renderHub::getScene(const int index)
{
	std::list<scene>::iterator iter = sceneList.begin();
	for( int i=0; i < index; ++i) {++iter;}
	return &(*iter);
}

void renderHub::setActiveScene(const int index)
{
	std::list<scene>::iterator iter = sceneList.begin();
	for( int i=0; i < index; ++i) {++iter;}
	activeScene = &(*iter);
}

void renderHub::run()
{
	running = true;

	/*	
	/	Just for testing and debug purposes I am ignoring the event-queue concept I want to take up later
	/	and manually add a single entity to the active scene
	*/
	activeScene->createStaticSceneObject(0,glm::vec3(0.0),glm::vec4(1.0,0.0,0.0,0.0));

	while(running)
	{
		activeScene->render();
		glfwSwapBuffers();
	}
}