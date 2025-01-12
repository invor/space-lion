#ifndef ProximityTriggerComponentManager_hpp
#define ProximityTriggerComponentManager_hpp

#include "functional"

#include "BaseMultiInstanceComponentManager2.hpp"

namespace EngineCore
{
    namespace Common {

        struct ProximityTriggerComponentData
        {
            Entity                entity;           ///< entity that owns the component
            Entity                target;           ///< entity for which the proximity is tracked
            float                 trigger_distance; ///< distance at which the callback will trigger
            bool                  in_proximity;     ///< flag whether target was in proximity during the last check
            std::function<void()> enter_callback;   ///< function that is called when the target enters proximity
            std::function<void()> leave_callback;   ///< function that is called when the target leaves proximity
        };

        class ProximityTriggerComponentManager : public BaseMultiInstanceComponentManager2<ProximityTriggerComponentData, 1000, 1000>
        {
        public:
            ProximityTriggerComponentManager() = default;
            ~ProximityTriggerComponentManager() = default;

            size_t addComponent(Entity entity, Entity target, float trigger_distance, std::function<void()> enter_callback, std::function<void()> leave_callback);
        };

    }
}

#endif // !ProximityTriggerComponentManager_hpp
