#ifndef PointlightComponent_h
#define PointlightComponent_h

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct PointlightComponentData
        {
            Entity entity;       ///< entity owning the component
            Vec3   light_colour; ///< color of the light in rgb values
            float  lumen;        ///< Luminous power of the light source given in Lumen (lm)
            float  radius;       ///< Maximum radius
        };

        class PointlightComponentManager : public BaseMultiInstanceComponentManager2<PointlightComponentData,1000,1000>
        {
        public:
            PointlightComponentManager() = default;
            ~PointlightComponentManager() = default;

            size_t addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

            Entity getEntity(size_t index) const;

            Vec3 getColour(size_t index) const;

            float getLumen(size_t index) const;

            float getRadius(size_t index) const;

            void setColour(size_t index, Vec3 colour);

            void setLumen(size_t index, float lumen);

            void setRadius(size_t index, float radius);
        };

    }
}

#endif