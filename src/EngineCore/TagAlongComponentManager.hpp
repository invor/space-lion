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
                Data(Entity entity, Entity target, Vec3 offset, float slerp_alpha, float deadzone)
                    : entity(entity), target(target), offset(offset), slerp_alpha(slerp_alpha), deadzone(deadzone) {}

                Entity entity; ///< The entity that gets tagged, i.e. the entity that follows the target
                Entity target; ///< The entity that gets followed
                Vec3   offset; ///< Additional offset from the target entity

                /**
                * Slerp_alpha == 0    No movement for the taggee
                * 0 < Slerp_alpha < 1 Moves the taggee in front of the tagger (the higher the value (max 1), the faster the pace)
                * Slerp_alpha == 1    Places the taggee permantly in front of the tagger
                */
                float slerp_alpha;

                float deadzone; ///< Distance from the target location at which the entity starts moving towards the target
            };

            std::vector<Data> m_tag_data;
            std::shared_mutex m_tag_data_access_mutex;

        public:
            TagAlongComponentManager() = default;
            ~TagAlongComponentManager() = default;

            /**
            * Tags an entity 'taggee', so that it moves with slerp_alpha pace towards the taggers' front
            * 
            * Slerp_alpha values outside of [0, 1] are going to be clamped to [0, 1]
            */
            void addComponent(Entity entity, Entity target, Vec3 offset, float slerp_alpha, float deadzone = 0.0f);

            // TOOD: deleteComponent

            std::vector<Data> getTagComponentDataCopy();
        };
    }
}

#endif // !TagAlongComponentManager_hpp
