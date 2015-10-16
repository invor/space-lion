#ifndef renderHub_h
#define renderHub_h
/*
/	In this class, all parts directly related to the rendering done on a single device come together.
/
/	OpenGL context, window management, scene management, aswell as communication with other engine modules
/	is handled here.
*/

#define TIMER 0

#include <vector>
#include "scene.h"
#include "postProcessor.h"
#include "../ResourceManager.h"
#include "oldcontrols.h"
#include "framebufferObject.h"
#include <GLFW/glfw3.h>

class RenderHub
{
public:
	RenderHub();
	~RenderHub();

	/*	Initialize OpenGL context and create a window */
	bool init();

	/*	Scene handling */
	bool addScene();
	bool deleteScene();
	bool setSceneParameters();
	Scene* getScene(const int index);
	void setActiveScene(const int index);
	Scene* getActiveScene();

	/* Render a frame of the active scene and check event-queue. */
	void run();

	/*	Test volume rendering */
	void runVolumeTest();

protected:
	ResourceManager resourceMngr;

	std::list<Scene> sceneList;

	GLFWwindow *activeWindow;
	Scene *activeScene;

	bool running;

	/*	Callback handling */
	static RenderHub *activeInstance;
	static void setActiveInstance(RenderHub *instance);
	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);
};

#endif
