#ifndef TagAlongComponentManager_hpp
#define TagAlongComponentManager_hpp

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseSingleInstanceComponentManager.hpp"
#include "EntityManager.hpp"

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
                Data(Entity entity, Entity target, Vec3 offset)
                    : entity(entity), target(target), offset(offset) {}

                Entity entity;
                Entity target;
                Vec3 offset;
            };

            std::vector<Data> m_data;
            std::shared_mutex m_dataAccess_mutex;

        public:
            TagAlongComponentManager() = default;
            ~TagAlongComponentManager() = default;

            void addComponent(Entity entity, Entity target, Vec3 offset);

            std::vector<Data> getComponentDataCopy();
        };
    }
}

#endif // !TagAlongComponentManager_hpp
