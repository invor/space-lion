#ifndef MoveToComponentManager_hpp
#define MoveToComponentManager_hpp

// space-lion includes
#include "BaseSingleInstanceComponentManager2.hpp"
#include "ComponentStorage.hpp"

namespace EngineCore
{
    namespace Animation
    {
        enum class Space
        {
            GLOBAL,
            LOCAL
        };

        struct MoveToComponentData
        {
            Entity entity;           ///< entity that owns the component
            Vec3   target_position;  ///< target position of the movement
            float  speed;            ///< movement speed in m/s
            Space  move_orientation; ///< space in which the movement occurs (local or global)
        };

        class MoveToComponentManager : public BaseSingleInstanceComponentManager2<MoveToComponentData, 1000, 1000>
        {
        public:
            MoveToComponentManager() = default;
            ~MoveToComponentManager() = default;

            size_t addComponent(Entity entity, Vec3 target_position, float speed = 1.0f, Space move_orientation = Space::LOCAL);

            void setTargetPosition(Entity entity, Vec3 target_position);

            void setTargetPosition(size_t index, Vec3 target_position);
        };

    }
}

#endif // !MoveToComponentManager_hpp
