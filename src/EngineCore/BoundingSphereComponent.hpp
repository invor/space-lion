#ifndef BoundingSphereComponent_hpp
#define BoundingSphereComponent_hpp

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct BoundingSphereComponentData
        {
            Entity entity; ///< entity owning the component
            float  radius; ///< Radius of the bounding sphere
        };

        class BoundingSphereComponentManager : public BaseMultiInstanceComponentManager2<BoundingSphereComponentData,1000,1000>
        {
        public:
            BoundingSphereComponentManager() = default;
            ~BoundingSphereComponentManager() = default;

            size_t addComponent(Entity entity, float radius);

            Entity getEntity(size_t index) const;

            float getRadius(size_t index) const;

            void setRadius(size_t index, float radius);
        };

    }
}

#endif // !BoundingSphereComponent_hpp