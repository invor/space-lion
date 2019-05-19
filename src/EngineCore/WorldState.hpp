#ifndef WorldState_hpp
#define WorldState_hpp

#include <future>

#include "AirplanePhysicsComponent.hpp"
#include "CameraComponent.hpp"
#include "EntityManager.hpp"
#include "TransformComponentManager.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "DebugNameComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"

#include "OpenGL/ResourceManager.hpp"

namespace EngineCore
{
	class WorldState
	{
	public:
		typedef Graphics::OpenGL::ResourceManager ResourceManager;

        WorldState(ResourceManager* resource_manager)
            : m_transform_manager(4096),
            m_material_manager(resource_manager),
            m_camera_manager(8),
            m_mesh_manager(resource_manager),
            m_airplane_physics_manager(128, *this) {}
		~WorldState() = default;

		EntityManager&                                       accessEntityManager();

		Common::TransformComponentManager&                   accessTransformManager();
		Common::NameComponentManager&                        accessNameManager();

        Graphics::CameraComponentManager&                    accessCameraComponentManager();
		Graphics::MaterialComponentManager<ResourceManager>& accessMaterialComponentManager();
		Graphics::MeshComponentManager<ResourceManager>&     accessMeshComponentManager();
		Graphics::RenderTaskComponentManager&                accessRenderTaskComponentManager();

		Physics::AirplanePhysicsComponentManager&            accessAirplanePhysicsComponentManager();

	private:
		EntityManager m_entity_manager;
		
		//TODO add component managers for this world
		Common::TransformComponentManager m_transform_manager;
		Common::NameComponentManager m_name_manager;

        Graphics::CameraComponentManager                    m_camera_manager;
		Graphics::MaterialComponentManager<ResourceManager> m_material_manager;
		Graphics::MeshComponentManager<ResourceManager>     m_mesh_manager;
		Graphics::RenderTaskComponentManager                m_render_task_manager;

		Physics::AirplanePhysicsComponentManager            m_airplane_physics_manager;
	};


	inline EntityManager& WorldState::accessEntityManager() {
		return m_entity_manager;
	}

	inline Common::TransformComponentManager& WorldState::accessTransformManager() {
		return m_transform_manager;
	}
	inline Common::NameComponentManager& WorldState::accessNameManager() {
		return m_name_manager;
	}

    inline Graphics::CameraComponentManager& WorldState::accessCameraComponentManager() {
        return m_camera_manager;
    }

	inline Graphics::MaterialComponentManager<Graphics::OpenGL::ResourceManager>& WorldState::accessMaterialComponentManager() {
		return m_material_manager;
	}
	inline Graphics::MeshComponentManager<Graphics::OpenGL::ResourceManager>& WorldState::accessMeshComponentManager() {
		return m_mesh_manager;
	}
	inline Graphics::RenderTaskComponentManager& WorldState::accessRenderTaskComponentManager() {
		return m_render_task_manager;
	}

	inline Physics::AirplanePhysicsComponentManager & WorldState::accessAirplanePhysicsComponentManager()
	{
		return m_airplane_physics_manager;
	}


}

#endif // !WorldState_hpp
