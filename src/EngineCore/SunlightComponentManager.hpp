#ifndef SunlightComponent_hpp
#define SunlightComponent_hpp

struct Entity;

#include "BaseMultiInstanceComponentManager2.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        struct SunlightComponentData
        {
            Entity entity;       ///< entity owning that owns the component
            Vec3   light_colour; ///< color of the light in rgb values
            float  lumen;        ///< Luminous power of the light source given in Lumen (lm)
            float  star_radius;  ///< Radius of the star (used to compute solid angle)
        };

        class SunlightComponentManager : public BaseMultiInstanceComponentManager2<SunlightComponentData,1000,1000>
        {
        public:
            SunlightComponentManager() = default;
            ~SunlightComponentManager() = default;

            size_t addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

            void setColour(size_t index, Vec3 colour);

            void setLumen(size_t index, float lumen);

            void setStarRadius(size_t index, float radius);

            Entity getEntity(size_t index) const;

            Vec3 getColour(size_t index) const;

            float getLumen(size_t index) const;

            float getStarRadius(size_t index) const;
        };

    }
}

#endif