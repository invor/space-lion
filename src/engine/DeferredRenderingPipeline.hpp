#ifndef DeferredRenderingPipeline_h
#define DeferredRenderingPipeline_h

#include "RenderJobs.hpp"
#include "ComputeJobs.hpp"
#include "ResourceManager.h"
#include "MTQueue.hpp"
#include "Controls.hpp"
#include "framebufferObject.h"
#include "shaderStorageBufferObject.h"

#include <GLFW\glfw3.h>

class DeferredRenderingPipeline
{
private:
	/**
	 * Graphics resource management.
	 */
	ResourceManager m_resource_mngr;

	/**
	 * Render jobs for a light prepass, i.e. determining all relevant lights per (screen)tile.
	 * All (point) light sources are organized as render jobs, rendering a simple bounding sphere
	 * per light.
	 */
	RenderJobManager m_lights_prepass;

	/**
	 * Render jobs for shadow mapping render passes.
	 * Should only contain object that one wants to cast shadows. Using smaller mesh data
	 * (w.r.t. vertex data size) is also advised.
	 */
	RenderJobManager m_shadow_map_pass;

	/**
	 * Render jobs for all visible objects in the scene with opaque surface.
	 */
	RenderJobManager m_geometry_pass;

	/**
	 * Render jobs for all translucent objects in the scene.
	 */
	RenderJobManager m_orderIndependentTransparency_pass;

	/*
	 * Resources for deferred rendering lighting pass 
	 */
	std::shared_ptr<Mesh> m_dfr_fullscreenQuad;
	std::shared_ptr<GLSLProgram> m_dfr_lighting_prgm;

	/*
	 * Resources for order independent transparency rendering
	 */
	std::shared_ptr<ShaderStorageBufferObject> m_oit_headBuffer;
	std::shared_ptr<ShaderStorageBufferObject> m_oit_sampleBuffer;
	GLuint m_oit_counterBuffer;

	/** Pointer to active window */
	GLFWwindow* m_active_window;

	Entity m_active_camera;
	std::vector<Entity> m_active_lightsources;

	/**
	 * Thread-safe queue used to request new render jobs
	 */
	MTQueue<RenderJobRequest> m_renderJobRequest_queue;

	MTQueue<ComputeJobRequest> m_computeJobRequest_queue;

	/*
	 * Pointers to relevant modules of the engine, e.g. the EntityManager and TransformManager
	 */
	EntityManager* m_entity_mngr;
	TransformComponentManager* m_transform_mngr;
	CameraComponentManager* m_camera_mngr;
	LightComponentManager* m_light_mngr;

	/** Gather necessary resources and add actual RenderJobs from requests */
	void processRenderJobRequest();
	/**Gather necessary resources and add actual ComputeJobs from requests */
	void processComputeJobRequest();

	/**
	 * Render objects with transparency
	 */
	void orderIndependentTransparencyPass();

	/**
	 * Render solid geometry to framebuffer
	 */
	void geometryPass();

	/**
	 * Compute scene lighting
	 */
	void lightingPass();

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);

public:
	DeferredRenderingPipeline(EntityManager* entity_mngr, TransformComponentManager* transform_mngr, CameraComponentManager* camera_mngr, LightComponentManager* light_mngr);
	~DeferredRenderingPipeline();

	void requestRenderJob(Entity entity,
							std::string material_path,
							std::string mesh_path,
							bool cast_shadow = false);

	void requestComputeJob(Entity entity,
							GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z,
							std::string compute_prgm,
							std::vector<std::string> textures_ids,
							std::vector<std::string> volume_ids,
							std::vector<std::string> ssbo_ids,
							bool oneshot_job);

	void addLightsource(Entity entity);

	void setActiveCamera(Entity entity);

	void run();
};

#endif