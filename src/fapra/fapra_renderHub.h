#ifndef fapra_renderHub_h
#define fapra_renderHub_h
/*
/	In this class, all parts directly related to the rendering done on a single device come together.
/
/	OpenGL context, window management, scene management, aswell as communication with other engine modules
/	is handled here.
*/

/*	std includes */
#include <vector>

/*	space-lion includes */
#include "../engine/core/renderHub.h"
#include "planetaryScene.h"

/*	external lib includes */
#include "GLFW/glfw3.h"

class FapraRenderHub : public RenderHub
{
public:
	FapraRenderHub(void);
	~FapraRenderHub(void);

	void renderActiveScene();

private:
	PlanetaryScene demo_scene;

	std::vector<FramebufferObject> framebufferList;

	FramebufferObject *activeFramebuffer;
};

#endif