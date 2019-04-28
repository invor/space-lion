#include "GlobalCoreComponents.hpp"

#ifndef DeferredRenderingPipeline_h
#define DeferredRenderingPipeline_h

#include <functional>

//#include "AtmosphereComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "VolumeComponent.hpp"
#include "InterfaceMeshComponent.hpp"
#include "SelectComponent.hpp"
#include "RenderJobs.hpp"
#include "ResourceManager.h"
#include "MTQueue.hpp"
#include "Controls.hpp"
#include "framebufferObject.h"
#include "shaderStorageBufferObject.h"
#include "SunlightComponent.hpp"

#include <GLFW\glfw3.h>

class DeferredRenderingPipeline
{
private:
	/**
	 * Render jobs for a light prepass, i.e. determining all relevant lights per (screen)tile.
	 * All (point) light sources are organized as render jobs, rendering a simple bounding sphere
	 * per light.
	 * Not used yet.
	 */
	RenderJobManager m_lights_prepass;
	
	/**
	 * SSBO to store a per screen tile array of relevant light sources.
	 * Number of screen tiles and number of maximum light sources per tile is fixed.
	 * Not used yet.
	 */
	std::shared_ptr<ShaderStorageBufferObject> m_lights_ssbo;

	/**
	 * Render jobs for shadow mapping render passes.
	 * Should only contain object that one wants to cast shadows. Using smaller mesh data
	 * (w.r.t. vertex data size) is also advised.
	 */
	RenderJobManager m_shadow_map_pass;

	/** Render jobs for all visible objects in the scene with opaque surface. */
	RenderJobManager m_staticMeshes_pass;

	/** Render jobs for simple volumetric objects */
	RenderJobManager m_volume_pass;

	/** Render jobs for all translucent objects in the scene. */
	RenderJobManager m_orderIndependentTransparency_pass;

	/** Render jobs for a seperate (optional) picking pass */
	RenderJobManager m_picking_pass;

	/** Render jobs for interface related objects, i.e. move-gizmo in editor mode */
	RenderJobManager m_interface_pass;

	/** Thread-safe queue for tasks that have to be executed on the render thread, but only a single time */
	MTQueue<std::function<void()>> m_singleExecution_tasks;

	/** Thread-safe queue for tasks that have to be executed on the render thread, during the geometry pass of each frame */
	mutable std::mutex m_perFrameGeometry_tasks_mutex; //TODO replace with MTList
	std::list<std::function<void()>> m_perFrameGeometry_tasks;

	/** Thread-safe queue for tasks that have to be executed on the render thread, during the atmosphere pass of each frame */
	mutable std::mutex m_perFrameAtmosphere_tasks_mutex; //TODO replace with MTList
	std::list<std::function<void()>> m_perFrameAtmosphere_tasks;

	/** The g-Buffer used by the rendering pipeline */
	std::unique_ptr<FramebufferObject> m_gBuffer;

	/** Framebuffer used for atmosphereic entities */
	std::unique_ptr<FramebufferObject> m_atmosphere_fbo;


	/** Fullscreen quad mesh for deferred rendering passes */
	std::shared_ptr<Mesh> m_fullscreenQuad;
	/** Deferred rendering lighting pass program */
	std::shared_ptr<GLSLProgram> m_lighting_prgm;

	std::shared_ptr<Mesh> m_atmosphere_boundingSphere;

	/* Resources for order independent transparency rendering */
	std::shared_ptr<ShaderStorageBufferObject> m_oit_headBuffer;
	std::shared_ptr<ShaderStorageBufferObject> m_oit_sampleBuffer;
	GLuint m_oit_counterBuffer;

	/** Pointer to active window */
	GLFWwindow* m_active_window;

	Entity m_active_camera;
	std::vector<Entity> m_active_lightsources;

	/** Check StaticMeshComponentManager for newly added components and add actual render jobs */
	void registerStaticMeshComponents();

	/** Check VolumetricComponentManager for newly added volume and actual render jobs */
	void registerVolumetricComponents();

	/** Check InterfaceMeshComponentManager for newly added components and add actual render jobs */
	void registerInterfaceMeshComponents();

	/** Check SelectableComponentManager for newly added components and add actual render jobs */
	//void registerSelectableCompontens();

	/** Check PointLightComponentManager for newly added components and add to active light source list */
	void registerLightComponents();

	/** Render objects with transparency */
	void orderIndependentTransparencyPass();

	/** Render solid geometry to framebuffer */
	void geometryPass();

	/** Render volumes */
	void volumePass();

	/** Render atmospheres */
	void atmospherePass();

	/** Compute scene lighting */
	void lightingPass();

	/** Render interface objects */
	void interfacePass();

	/** Do a picking pass */
	void pickingPass();

	/** Process single execution task queue */
	void processSingleExecTasks();

	/** Process per frame geometry tasks */
	void processPerFrameGeometryTasks();

	/**
	 * TODO
	 * Compute new exposure from previous frame
	 */
	void computeExposure();


	/**************************************************************************
	 * (Static) callbacks functions
	 *************************************************************************/

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void windowCloseCallback(GLFWwindow* window);

public:
	DeferredRenderingPipeline();

	~DeferredRenderingPipeline();

	/** Start and run rendering pipeline. Returns only after rendering window is closed. */
	void run();

	/** Add a task to the render pipeline that only has to be executed once */
	void addSingleExecutionGpuTask(std::function<void()> task);

	/** Add a task to the render pipline that is executed each frame during the geometry pass */
	void addPerFrameGeometryGpuTask(std::function<void()> task);

	/** Add a task to the render pipline that is executed each frame during the atmosphere pass */
	void addPerFrameAtmosphereGpuTask(std::function<void()> task);

	void setActiveCamera(Entity entity);

	/** Return pointer to scene depth buffer (useful for volumetric components) */
	FramebufferObject* getGBuffer();

	Entity getActiveCamera();

};

#endif