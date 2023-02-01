#ifndef BoundingBoxComponent_hpp
#define BoundingBoxComponent_hpp

#include "EntityManager.hpp"
#include "BaseMultiInstanceComponentManager.hpp"
#include "types.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        class BoundingBoxComponentManager : public BaseMultiInstanceComponentManager
        {
        public:
            BoundingBoxComponentManager(uint size);
            ~BoundingBoxComponentManager();

            enum class BBAlignment {
                AXIS_ALIGNED,
                OBJECT_ALIGNED
            };

            void reallocate(uint size);

            void addComponent(Entity entity, float width, float height, float depth, BBAlignment alignment);

            void deleteComponent(Entity entity);

            uint getComponentCount() const;

            Entity getEntity(uint index) const;

            float getWidth(uint index) const;

            float getHeight(uint index) const;

            float getDepth(uint index) const;

            BBAlignment getAlignment(uint index) const;

            void setWidth(uint index, float width);

            void setHeight(uint index, float height);

            void setDepth(uint index, float depth);

            void setAlignment(uint index, BBAlignment alignment);

            std::vector<Entity> getListOfEntities() const;

        private:
            struct Data
            {
                uint         used;      ///< number of components currently in use
                uint         allocated; ///< number of components that the allocated memory can hold
                uint8_t*     buffer;    ///< raw data pointer

                Entity*      entity;    ///< entity owning the component
                float*       width;     ///< Width of the bounding box
                float*       height;    ///< Height of the bounding box
                float*       depth;     ///< Depth of the bounding box
                BBAlignment* alignment; ///< Alignment of the bounding box (axis- or object-aligned)
            };

            Data m_data;
        };

    }
}

#endif // !BoundingBoxComponent_hpp