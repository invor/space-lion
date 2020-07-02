#ifndef WorldState_hpp
#define WorldState_hpp

#include <future>

#include "AirplanePhysicsComponent.hpp"
#include "CameraComponent.hpp"
#include "EntityManager.hpp"
#include "gltfAssetComponentManager.hpp"
#include "TransformComponentManager.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "NameComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"
#include "TurntableComponentManager.hpp"

#include "OpenGL/ResourceManager.hpp"

namespace EngineCore
{
    class WorldState
    {
    public:
        typedef Graphics::OpenGL::ResourceManager ResourceManager;

        WorldState() = default;
        ~WorldState() = default;

        EntityManager&                                        accessEntityManager();

        /**
         *
         */
        template <typename ComponentType>
        ComponentType& get() {
            auto it = m_component_managers.find(getTypeId<ComponentType>());
            assert(it != m_component_managers.end());
            return (*(static_cast<ComponentType*>(it->second.get())));
        }

        /**
         *
         */
        template <class ComponentType>
        void registerComponentManager(std::unique_ptr<BaseComponentManager> &&value) {
            m_component_managers.emplace(getTypeId<ComponentType>() , std::forward<std::unique_ptr<BaseComponentManager>>(value) );
        }

    private:
        EntityManager m_entity_manager;

        /**
         * Type map, inspired by https://gpfault.net/posts/mapping-types-to-values.txt.html
         */
        std::unordered_map<int, std::unique_ptr<BaseComponentManager>> m_component_managers;

        template <class ComponentType>
        inline static int getTypeId() {
            static const int id = last_type_id++;
            return id;
        }

        static std::atomic_int last_type_id;
    };

    inline EntityManager& WorldState::accessEntityManager() {
        return m_entity_manager;
    }

}

#endif // !WorldState_hpp
