#ifndef CooldownTriggerComponentManager_hpp
#define CooldownTriggerComponentManager_hpp

#include "functional"

#include "BaseMultiInstanceComponentManager2.hpp"
#include "ComponentStorage.hpp"

namespace EngineCore {
    namespace Common {

        struct CooldownTriggerComponentData
        {
            Entity                entity;            ///< entity that owns the component
            bool                  is_active;         ///< flag denoting if cooldown actively counts down
            float                 remaining_time;    ///< remaining cooldown time
            float                 reset_time;        ///< value that the cooldown is reset to
            std::function<void()> cooldown_callback; ///< function that is called when the cooldown time has passed
        };

        class CooldownTriggerComponentManager : public BaseMultiInstanceComponentManager2<CooldownTriggerComponentData, 1000, 1000>
        {
        public:
            CooldownTriggerComponentManager() = default;
            ~CooldownTriggerComponentManager() = default;

            size_t addComponent(Entity entity, float reset_time, std::function<void()> cooldown_callback, bool start_on_creation = true);

            void resetCooldown(Entity entity);

            void resetCooldown(size_t index);
        };

    }
}

#endif // !CooldownTriggerComponentManager_hpp
