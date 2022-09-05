#ifndef SunlightComponent_hpp
#define SunlightComponent_hpp

struct Entity;

#include "BaseMultiInstanceComponentManager.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        class SunlightComponentManager : public BaseMultiInstanceComponentManager
        {
        private:
            struct Data
            {
                uint     used;      ///< number of components currently in use
                uint     allocated; ///< number of components that the allocated memery can hold
                uint8_t* buffer;    ///< raw data pointer

                Entity* entity;       ///< entity owning that owns the component
                Vec3*   light_colour; ///< color of the light in rgb values
                float*  lumen;        ///< Luminous power of the light source given in Lumen (lm)
                float*  star_radius;  ///< Radius of the star (used to compute solid angle)
            };

            Data m_data;

        public:
            SunlightComponentManager(uint size);
            ~SunlightComponentManager();

            void reallocate(uint size);

            void addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

            void deleteComonent(Entity entity);

            uint getComponentCount() const { return m_data.used; }

            void setColour(uint index, Vec3 colour);

            void setLumen(uint index, float lumen);

            void setStarRadius(uint index, float radius);

            Entity getEntity(uint index) const;

            Vec3 getColour(uint index) const;

            float getLumen(uint index) const;

            float getStarRadius(uint index) const;
        };

    }
}

#endif