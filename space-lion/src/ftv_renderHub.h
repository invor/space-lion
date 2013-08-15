#ifndef ftv_renderHub_h
#define ftv_renderHub_h
/*
/	In this class, all parts directly related to the rendering done on a single device come together.
/
/	OpenGL context, window management, scene management, aswell as communication with other engine modules
/	is handled here.
*/

#define TIMER 0

#include <vector>
#include "scene.h"
#include "ftv_scene.h"
#include "postProcessor.h"
#include "resourceManager.h"
#include "ftv_postProcessor.h"
#include "framebufferObject.h"
#include "ftvTestbench.h"
#include "GL/glfw.h"

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"GLFW.lib")
	#pragma comment(lib,"opengl32.lib")
#endif

class Ftv_RenderHub
{
public:
	Ftv_RenderHub(void);
	~Ftv_RenderHub(void);

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

	/*	Run fault tolerant volume visuailization tests. */
	void runFtvVolumeTest();

	/*	Run fault tolerant visualization tests. */
	void runFtv();

	/*	Used for crazy testing */
	void runInpaintingTest();
	void runFtvGuidanceFieldTest();

private:
	ResourceManager resourceMngr;

	std::vector<FramebufferObject> framebufferList;
	std::list<Scene> sceneList;

	FramebufferObject *activeFramebuffer;
	Scene *activeScene;

	bool running;
};

#endif
