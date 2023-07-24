#ifndef BillboardComponentManager_hpp
#define BillboardComponentManager_hpp

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseSingleInstanceComponentManager.hpp"
#include "EntityManager.hpp"

// TODO: documentation

namespace EngineCore
{
    class WorldState;

    namespace Animation
    {
        /**
        * Entities with billboard component will turn their z axis towards the designated target entity
        */
        class BillboardComponentManager : public BaseSingleInstanceComponentManager
        {
        private:
            struct Data
            {
                Data(Entity entity, Entity target)
                    : entity(entity), target(target) {}

                Entity entity; ///< The entity that gets rotated towards the target
                Entity target; ///< The entity that will be faced by the billboard
            };

            std::vector<Data> m_billboard_data;
            std::shared_mutex m_billboard_data_access_mutex;

        public:
            BillboardComponentManager() = default;
            ~BillboardComponentManager() = default;

            void addComponent(Entity entity, Entity target);

            // TODO: deleteComponent??

            std::vector<Data> getBillboardComponentDataCopy();
        };
    }
}

#endif // !BillboardComponentManager_hpp
