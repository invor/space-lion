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
    /**
     * The state of a world instance. Made up from storage and management of all entities and all components.
     */
    class WorldState
    {
    public:
        typedef Graphics::OpenGL::ResourceManager ResourceManager;

        WorldState() = default;
        ~WorldState() = default;

        EntityManager& accessEntityManager();

        /**
         *
         */
        template <typename ComponentManagerType>
        ComponentManagerType& get();

        /**
         *
         */
        template <class ComponentManagerType>
        void add(std::unique_ptr<BaseComponentManager> &&value);

    private:
        /**
         * Entity manager for storing and managing all entities of a world.
         */
        EntityManager m_entity_manager;

        /**
         * Type map for flexible storage of all component managers (inspired by https://gpfault.net/posts/mapping-types-to-values.txt.html)
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

    template <typename ComponentManagerType>
    inline ComponentManagerType& WorldState::get()
    {
        auto it = m_component_managers.find(getTypeId<ComponentManagerType>());
        assert(it != m_component_managers.end());
        return (*(static_cast<ComponentManagerType*>(it->second.get())));
    }

    template <class ComponentManagerType>
    inline void WorldState::add(std::unique_ptr<BaseComponentManager> &&value)
    {
        m_component_managers.emplace(getTypeId<ComponentManagerType>(), std::forward<std::unique_ptr<BaseComponentManager>>(value));
    }

}

#endif // !WorldState_hpp
