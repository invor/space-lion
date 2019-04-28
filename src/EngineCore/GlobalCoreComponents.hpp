#ifndef GlobalCoreComponents_hpp
#define GlobalCoreComponents_hpp

#include <memory>

#include "CameraComponent.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "EntityManager.hpp"
#include "InterfaceMeshComponent.hpp"
#include "PointlightComponent.hpp"
#include "ResourceManager.h"
#include "StaticMeshComponent.hpp"
#include "SunlightComponent.hpp"
#include "TransformComponent.hpp"
#include "VolumeComponent.hpp"

namespace GCoreComponents
{
	EntityManager& entityManager();
	TransformComponentManager& transformManager();
	DeferredRenderingPipeline& renderingPipeline();
	ResourceManager& resourceManager();

	CameraComponentManager& cameraManager();
	SunlightComponentManager& sunlightManager();
	PointlightComponentManager& pointlightManager();

	StaticMeshComponentManager& staticMeshManager();
	InterfaceMeshComponentManager& interfaceMeshManager();
	VolumeComponentManager& volumeManager();
}

#endif // !GlobalCoreComponents_hpp
