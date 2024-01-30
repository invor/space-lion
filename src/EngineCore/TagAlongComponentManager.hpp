#ifndef TagAlongComponentManager_hpp
#define TagAlongComponentManager_hpp

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
        class TagAlongComponentManager : public BaseSingleInstanceComponentManager
        {
        private:
            struct Data
            {
                Data(Entity entity, Entity target, Vec3 offset, float time_to_target, float deadzone)
                    : entity(entity), target(target), offset(offset), time_to_target(time_to_target), deadzone(deadzone) {}

                Entity entity; ///< The entity that gets tagged, i.e. the entity that follows the target
                Entity target; ///< The entity that gets followed
                Vec3   offset; ///< Additional offset from the target entity
                float  time_to_target; ///< Time in seconds needed to reach the target (or deadzone)
                float  deadzone; ///< Distance from the target location at which the entity starts moving towards the target
            };

            std::vector<Data> m_tag_data;
            std::shared_mutex m_tag_data_access_mutex;

        public:
            TagAlongComponentManager() = default;
            ~TagAlongComponentManager() = default;

            void addComponent(Entity entity, Entity target, Vec3 offset, float time_to_target, float deadzone = 0.0f);

            // TOOD: deleteComponent

            void setTarget(Entity entity, Entity target);

            std::vector<Data> getTagComponentDataCopy();
        };
    }
}

#endif // !TagAlongComponentManager_hpp
