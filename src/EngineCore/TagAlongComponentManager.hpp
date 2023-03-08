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
                Data(Entity tagger, Entity taggee, Vec3 offset)
                    : tagger(tagger), taggee(taggee), offset(offset) {}

                Entity tagger; // The entity that gets followed by the taggee
                Entity taggee; // The entity that gets tagged, i.e. the entity that follows the tagger

                // Instead of offset, it might be useful to define a minimum distance between both entities
                Vec3 offset;
            };

            std::vector<Data> m_tag_data;
            std::shared_mutex m_tag_data_access_mutex;

            std::vector<Data> m_hud_data;
            std::shared_mutex m_hud_data_access_mutex;

        public:
            TagAlongComponentManager() = default;
            ~TagAlongComponentManager() = default;

            void addTagComponent(Entity tagger, Entity taggee, Vec3 offset);
            void addHUDComponent(Entity tagger, Entity taggee, Vec3 offset);

            std::vector<Data> getTagComponentDataCopy();
            std::vector<Data> getHUDComponentDataCopy();
        };
    }
}

#endif // !TagAlongComponentManager_hpp
