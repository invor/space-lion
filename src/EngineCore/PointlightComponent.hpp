#ifndef PointlightComponent_h
#define PointlightComponent_h

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        class PointlightComponentManager : public BaseMultiInstanceComponentManager
        {
        private:
            struct Data
            {
                uint     used;      ///< number of components currently in use
                uint     allocated; ///< number of components that the allocated memory can hold
                uint8_t* buffer;    ///< raw data pointer

                Entity* entity;       ///< entity owning the component
                Vec3*   light_colour; ///< color of the light in rgb values
                float*  lumen;        ///< Luminous power of the light source given in Lumen (lm)
                float*  radius;       ///< Maximum radius
            };

            Data m_data;
            mutable std::shared_mutex m_data_access_mutex;

        public:
            PointlightComponentManager(uint size);
            ~PointlightComponentManager();

            void reallocate(uint size);

            void addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

            void deleteComponent(Entity entity);

            uint getComponentCount() const;

            Entity getEntity(uint index) const;

            Vec3 getColour(uint index) const;

            float getLumen(uint index) const;

            float getRadius(uint index) const;

            void setColour(uint index, Vec3 colour);

            void setLumen(uint index, float lumen);

            void setRadius(uint index, float radius);

            std::vector<Entity> getListOfEntities() const;
        };

    }
}

#endif