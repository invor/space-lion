#ifndef TurntableComponentManager_hpp
#define TurntableComponentManager_hpp

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
        class TurntableComponentManager : public BaseSingleInstanceComponentManager
        {
        public:
            struct Data
            {
                Data(Entity entity, float angle, Vec3 axis)
                    : entity(entity), angle(angle), axis(axis) {}

                Entity entity;
                float  angle;
                Vec3   axis;
            };
        private:
            std::vector<Data> m_data;
            std::shared_mutex m_dataAccess_mutex;
            
        public:
            TurntableComponentManager() = default;
            ~TurntableComponentManager() = default;

            void addComponent(Entity entity, float angle, Vec3 axis = Vec3(0.0f,1.0f,0.0f));

            std::vector<Data> getComponentDataCopy();
        };
    }
}

#endif // !TurntableComponentManager_hpp
