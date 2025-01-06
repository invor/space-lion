#ifndef TagAlongComponentManager_hpp
#define TagAlongComponentManager_hpp

#include "BaseSingleInstanceComponentManager2.hpp"
#include "EntityManager.hpp"

// TODO: documentation

namespace EngineCore
{
    class WorldState;

    namespace Animation
    {

        struct TagAlongComponentData
        {
            Entity entity; ///< The entity that gets tagged, i.e. the entity that follows the target
            Entity target; ///< The entity that gets followed
            Vec3   offset; ///< Additional offset from the target entity
            float  time_to_target; ///< Time in seconds needed to reach the target (or deadzone)
            float  deadzone; ///< Distance from the target location at which the entity starts moving towards the target
        };

        class TagAlongComponentManager : public BaseSingleInstanceComponentManager2<TagAlongComponentData, 1000, 1000>
        {
        public:
            TagAlongComponentManager() = default;
            ~TagAlongComponentManager() = default;

            size_t addComponent(Entity entity, Entity target, Vec3 offset, float time_to_target, float deadzone = 0.0f);

            void setTarget(Entity entity, Entity target);
        };
    }
}

#endif // !TagAlongComponentManager_hpp
