#include "GlobalCoreComponents.hpp"

namespace GCoreComponents
{
	namespace
	{
		EntityManager g_entity_mngr;
		CameraComponentManager g_camera_mngr(24);
		TransformComponentManager g_transform_mngr(100000);

		DeferredRenderingPipeline g_rendering_pipeline;
		ResourceManager g_resource_mngr;

		SunlightComponentManager g_sunlight_mngr(12);
		PointlightComponentManager g_pointlight_mngr(2000);

		StaticMeshComponentManager g_staticMesh_mngr;
		InterfaceMeshComponentManager g_interfaceMesh_mngr;
		VolumeComponentManager g_volume_mngr;
	}

	EntityManager& entityManager()
	{
		return g_entity_mngr;
	}

	TransformComponentManager& transformManager()
	{
		return g_transform_mngr;
	}

	DeferredRenderingPipeline& renderingPipeline()
	{
		return g_rendering_pipeline;
	}

	ResourceManager& resourceManager()
	{
		return g_resource_mngr;
	}

	CameraComponentManager& cameraManager()
	{
		return g_camera_mngr;
	}

	SunlightComponentManager& sunlightManager()
	{
		return g_sunlight_mngr;
	}

	PointlightComponentManager& pointlightManager()
	{
		return g_pointlight_mngr;
	}

	StaticMeshComponentManager& staticMeshManager()
	{
		return g_staticMesh_mngr;
	}

	InterfaceMeshComponentManager& interfaceMeshManager()
	{
		return g_interfaceMesh_mngr;
	}

	VolumeComponentManager& volumeManager()
	{
		return g_volume_mngr;
	}
}