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
        class BillboardComponentManager : public BaseSingleInstanceComponentManager
        {
        private:
            struct Data
            {
                Data(Entity tagger, Entity taggee)
                    : tagger(tagger), taggee(taggee) {}

                Entity tagger; // The entity that gets followed by the taggee
                Entity taggee; // The entity that gets tagged, i.e. the entity that follows the tagger
            };

            std::vector<Data> m_billboard_data;
            std::shared_mutex m_billboard_data_access_mutex;

        public:
            BillboardComponentManager() = default;
            ~BillboardComponentManager() = default;

            void addComponent(Entity tagger, Entity taggee);

            // TODO: deleteComponent??

            std::vector<Data> getBillboardComponentDataCopy();
        };
    }
}

#endif // !BillboardComponentManager_hpp
