#ifndef BoundingBoxComponent_hpp
#define BoundingBoxComponent_hpp

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        enum class BBAlignment {
            AXIS_ALIGNED,
            OBJECT_ALIGNED
        };

        struct BoundingBoxData
        {
            Entity      entity; ///< entity owning the component
            Vec3        min;
            Vec3        max;
            BBAlignment alignment; ///< Alignment of the bounding box (axis- or object-aligned)
        };

        class BoundingBoxComponentManager : public BaseMultiInstanceComponentManager2<BoundingBoxData,1000,1000>
        {
        public:
            BoundingBoxComponentManager() = default;
            ~BoundingBoxComponentManager() = default;

            size_t addComponent(Entity entity, Vec3 min, Vec3 max, BBAlignment alignment);

            BBAlignment getAlignment(size_t index) const;

            void setAlignment(size_t index, BBAlignment alignment);
        };

    }
}

#endif // !BoundingBoxComponent_hpp