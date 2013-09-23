#ifndef ftv_renderHub_h
#define ftv_renderHub_h
/*
/	In this class, all parts directly related to the rendering done on a single device come together.
/
/	OpenGL context, window management, scene management, aswell as communication with other engine modules
/	is handled here.
*/

#define TIMER 1

#include <vector>
#include "scene.h"
#include "ftv_scene.h"
#include "postProcessor.h"
#include "resourceManager.h"
#include "ftv_postProcessor.h"
#include "ftv_resourceManager.h"
#include "framebufferObject.h"
#include "ftvTestbench.h"
#include "renderHub.h"
#include "GLFW/glfw3.h"

class Ftv_RenderHub : public RenderHub
{
public:
	Ftv_RenderHub(void);
	~Ftv_RenderHub(void);

	/*	Run fault tolerant volume visuailization tests. */
	void runFtvVolumeTest();

	/*	Run fault tolerant visualization tests. */
	void runFtv();

	/*	Used for crazy testing */
	void runInpaintingTest();
	void runTextureAdvectionTest();
	void runFtvGuidanceFieldTest();

private:
	Ftv_ResourceManager resourceMngr;

	std::vector<FramebufferObject> framebufferList;
	std::list<Scene> sceneList;

	FramebufferObject *activeFramebuffer;
	Scene *activeScene;

	bool running;
};

#endif
