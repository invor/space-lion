#ifndef BillboardComponentManager_hpp
#define BillboardComponentManager_hpp

#include "BaseSingleInstanceComponentManager2.hpp"
#include "EntityManager.hpp"

// TODO: documentation

namespace EngineCore
{
    class WorldState;

    namespace Animation
    {
        struct BillboardComponentData
        {
            Entity entity; ///< The entity that gets rotated towards the target
            Entity target; ///< The entity that will be faced by the billboard
        };

        /**
        * Entities with billboard component will turn their z axis towards the designated target entity
        */
        class BillboardComponentManager : public BaseSingleInstanceComponentManager2<BillboardComponentData, 1000, 1000>
        {
        public:
            BillboardComponentManager() = default;
            ~BillboardComponentManager() = default;

            size_t addComponent(Entity entity, Entity target);
        };
    }
}

#endif // !BillboardComponentManager_hpp
