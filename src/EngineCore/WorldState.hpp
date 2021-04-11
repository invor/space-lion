#ifndef WorldState_hpp
#define WorldState_hpp

#include <future>

#include "BaseComponentManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    /**
     * The state of a world instance. Made up from storage and management of all entities and all components.
     */
    class WorldState
    {
    public:

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
        void add(std::unique_ptr<BaseComponentManager> &&component_mngr);

        /** 
         *
         */
        void add(std::function<void(WorldState&, double)> system);

        std::vector<std::function<void(WorldState&, double)>> const& getSystems();

    private:
        /**
         * Entity manager for storing and managing all entities of a world.
         */
        EntityManager m_entity_manager;

        /**
         * Type map for flexible storage of all component managers
         * (inspired by https://gpfault.net/posts/mapping-types-to-values.txt.html)
         */
        std::unordered_map<int, std::unique_ptr<BaseComponentManager>> m_component_managers;

        /** 
         *
         */
        std::vector<std::function<void(WorldState&, double)>> m_systems;

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

    inline void WorldState::add(std::function<void(WorldState&, double)> system)
    {
        m_systems.emplace_back(system);
    }

    inline std::vector<std::function<void(WorldState&, double)>> const & WorldState::getSystems()
    {
        return m_systems;
    }

    template <typename ComponentManagerType>
    inline ComponentManagerType& WorldState::get()
    {
        auto it = m_component_managers.find(getTypeId<ComponentManagerType>());
        assert(it != m_component_managers.end());
        return (*(static_cast<ComponentManagerType*>(it->second.get())));
    }

    template <class ComponentManagerType>
    inline void WorldState::add(std::unique_ptr<BaseComponentManager> &&component_mngr)
    {
        m_component_managers.emplace(
            getTypeId<ComponentManagerType>(), 
            std::forward<std::unique_ptr<BaseComponentManager>>(component_mngr)
        );
    }

}

#endif // !WorldState_hpp
