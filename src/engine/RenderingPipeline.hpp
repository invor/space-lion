#ifndef RenderingPipeline_h
#define RenderingPipeline_h

#include "RenderJobs.hpp"
#include "ResourceManager.h"
#include "MTQueue.hpp"

#include <GLFW\glfw3.h>

class RenderingPipeline
{
private:
	/**
	 * Graphics resource management.
	 */
	ResourceManager m_resource_mngr;

	RenderJobManager m_shadow_map_pass;

	RenderJobManager m_forward_render_pass;

	GLFWwindow* m_active_window;

	Entity m_active_camera;

	MTQueue<RenderJobRequest> m_jobRequest_queue;

	EntityManager* m_entity_mngr;
	TransformComponentManager* m_transform_mngr;

	void processRenderJobRequest();

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);

public:
	RenderingPipeline(EntityManager* entity_mngr, TransformComponentManager* transform_mngr, CameraComponentManager* camera_mngr);
	~RenderingPipeline();

	void requestRenderJob(Entity entity, std::string material_path, std::string mesh_path, bool cast_shadow = false);

	void run();
};

#endif