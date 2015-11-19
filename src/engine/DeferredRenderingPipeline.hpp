#ifndef DeferredRenderingPipeline_h
#define DeferredRenderingPipeline_h

#include "AtmosphereComponent.hpp"
#include "RenderJobs.hpp"
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

	/** Render jobs for all visible objects in the scene with opaque surface. */
	RenderJobManager m_geometry_pass;

	/** Render jobs for all visible planetary atmospheres */
	RenderJobManager m_atmosphere_pass;

	/** Render jobs for all translucent objects in the scene. */
	RenderJobManager m_orderIndependentTransparency_pass;

	/*
	 * Resources for deferred rendering lighting pass 
	 */
	std::shared_ptr<Mesh> m_dfr_fullscreenQuad;
	std::shared_ptr<GLSLProgram> m_dfr_lighting_prgm; // TODO remove dfr naming...this already is within a Deferred Rendering class

	std::shared_ptr<Mesh> m_atmosphere_boundingBox;

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

	/** Thread-safe queue used to request new render jobs */
	MTQueue<RenderJobRequest> m_renderJobRequest_queue;

	/**************************************************************************
	 * Pointers to relevant modules of the engine,
	 * e.g. the EntityManager and TransformManager
	 *************************************************************************/
	EntityManager* m_entity_mngr;
	ResourceManager* m_resource_mngr;
	TransformComponentManager* m_transform_mngr;
	CameraComponentManager* m_camera_mngr;
	LightComponentManager* m_light_mngr;
	AtmosphereComponentManager* m_atmosphere_mngr;

	/** Gather necessary resources and add actual RenderJobs from requests */
	void processRenderJobRequest();

	/**
	 * Render objects with transparency
	 */
	void orderIndependentTransparencyPass();

	/**
	 * Render solid geometry to framebuffer
	 */
	void geometryPass();

	/**
	 * Render atmospheres
	 */
	void atmospherePass();

	/**
	 * Compute scene lighting
	 */
	void lightingPass();


	/**************************************************************************
	 * (Static) callbacks functions
	 *************************************************************************/

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);

public:
	DeferredRenderingPipeline(EntityManager* entity_mngr,
							ResourceManager* resource_mngr,
							TransformComponentManager* transform_mngr,
							CameraComponentManager* camera_mngr, 
							LightComponentManager* light_mngr,
							AtmosphereComponentManager* atmosphere_mngr);

	~DeferredRenderingPipeline();

	void requestRenderJob(const RenderJobRequest& new_request);

	void addLightsource(Entity entity);

	void setActiveCamera(Entity entity);

	void run();
};

#endif