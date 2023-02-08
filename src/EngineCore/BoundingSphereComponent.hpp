#ifndef BoundingSphereComponent_hpp
#define BoundingSphereComponent_hpp

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        class BoundingSphereComponentManager : public BaseMultiInstanceComponentManager
        {
        public:
            BoundingSphereComponentManager(uint size);
            ~BoundingSphereComponentManager();

            void reallocate(uint size);

            void addComponent(Entity entity, float radius);

            void deleteComponent(Entity entity);

            uint getComponentCount() const;

            Entity getEntity(uint index) const;

            float getRadius(uint index) const;

            void setRadius(uint index, float radius);

            std::vector<Entity> getListOfEntities() const;

        private:
            struct Data
            {
                uint     used;      ///< number of components currently in use
                uint     allocated; ///< number of components that the allocated memory can hold
                uint8_t* buffer;    ///< raw data pointer

                Entity* entity;     ///< entity owning the component
                float* radius;      ///< Radius of the bounding sphere
            };

            Data m_data;
            mutable std::shared_mutex m_data_access_mutex;
        };

    }
}

#endif // !BoundingSphereComponent_hpp