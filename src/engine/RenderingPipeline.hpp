#ifndef RenderingPipeline_h
#define RenderingPipeline_h

#include "RenderJobs.hpp"
#include "ResourceManager.h"
#include "MTQueue.hpp"
#include "Controls.hpp"

#include <GLFW\glfw3.h>

class RenderingPipeline
{
private:
	/**
	 * Graphics resource management.
	 */
	ResourceManager m_resource_mngr;

	/*
	 * Render jobs for a light prepass, i.e. determining all relevant lights per (screen)tile.
	 * All (point) light sources are organized as render jobs, rendering a simple bounding sphere
	 * per light.
	 */
	RenderJobManager m_lights_prepass;

	RenderJobManager m_shadow_map_pass;

	RenderJobManager m_forward_render_pass;

	GLFWwindow* m_active_window;

	Entity m_active_camera;
	std::vector<Entity> m_active_lightsources;

	/*
	 * Thread-safe queue used to request new render jobs
	 */
	MTQueue<RenderJobRequest> m_jobRequest_queue;

	/*
	 * Pointers to relevant modules of the engine, e.g. the EntityManager and TransformManager
	 */
	EntityManager* m_entity_mngr;
	TransformComponentManager* m_transform_mngr;

	void processRenderJobRequest();

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);

public:
	RenderingPipeline(EntityManager* entity_mngr, TransformComponentManager* transform_mngr, CameraComponentManager* camera_mngr, LightComponentManager* light_mngr);
	~RenderingPipeline();

	void requestRenderJob(Entity entity, std::string material_path, std::string mesh_path, bool cast_shadow = false);

	void addLightsource(Entity entity);

	void run();
};

#endif