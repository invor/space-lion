#ifndef BoundingCylinderComponent_hpp
#define BoundingCylinderComponent_hpp

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct BoundingCylinderComponentData
        {
            Entity entity; ///< entity owning the component
            float  radius; ///< Radius of the bounding cylinder
            float  height; ///< height of the bounding cylinder
        };

        class BoundingCylinderComponentManager : public BaseMultiInstanceComponentManager2<BoundingCylinderComponentData,1000,1000>
        {
        public:
            BoundingCylinderComponentManager() = default;
            ~BoundingCylinderComponentManager() = default;

            size_t addComponent(Entity entity, float radius, float height);

            Entity getEntity(uint index) const;

            float getRadius(uint index) const;

            float getHeight(uint index) const;

            void setRadius(uint index, float radius);

            void setHeight(uint index, float height);
        };

    }
}

#endif // !BoundingCylinderComponent_hpp