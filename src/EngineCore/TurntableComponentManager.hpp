#ifndef TurntableComponentManager_hpp
#define TurntableComponentManager_hpp

#include "BaseSingleInstanceComponentManager2.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    class WorldState;

    namespace Animation
    {
        struct TurntableComponentData
        {
            Entity entity;
            float  angle;
            Vec3   axis;
        };

        class TurntableComponentManager : public BaseSingleInstanceComponentManager2<TurntableComponentData, 1000, 1000>
        {
        public:
            TurntableComponentManager() = default;
            ~TurntableComponentManager() = default;

            size_t addComponent(Entity entity, float angle, Vec3 axis = Vec3(0.0f,1.0f,0.0f));
        };
    }
}

#endif // !TurntableComponentManager_hpp
