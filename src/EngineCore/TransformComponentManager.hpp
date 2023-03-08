#ifndef TransformComponent_h
#define TransformComponent_h

// space-lion includes
#include "BaseSingleInstanceComponentManager.hpp"
#include "EntityManager.hpp"
#include "types.hpp"

// std includes
#include <unordered_map>
#include <iostream>
#include <shared_mutex>

namespace EngineCore
{
    namespace Common
    {
        class TransformComponentManager : public BaseSingleInstanceComponentManager
        {
        private:
            struct Data
            {
                size_t used;					///< number of components currently in use
                size_t allocated;				///< number of components that the allocated memery can hold
                uint8_t* buffer;			///< raw data pointer

                Entity* entity;				///< entity that owns the component
                Mat4x4* world_transform;	///< the actual transformation (aka model matrix)
                Vec3* position;				///< local position (equals global position if component has no parent)
                Quat* orientation;			///< local orientation (...)
                Vec3* scale;				///< local scale (...)

                size_t* parent;				///< index to parent (equals components own index if comp. has no parent)
                size_t* first_child;			///< index to child (...)
                size_t* next_sibling;			///< index to sibling (...)
            };

            Data m_data;
            mutable std::shared_mutex m_data_access_mutex;
            
            void transform(size_t index);

        public:
            TransformComponentManager();
            TransformComponentManager(size_t size);
            ~TransformComponentManager();

            void reallocate(size_t size);

            size_t addComponent(Entity entity, Vec3 position = Vec3(), Quat orientation = Quat(), Vec3 scale = Vec3(1.0));

            void deleteComonent(Entity entity);

            size_t getComponentCount() const;

            void translate(Entity entity, Vec3 translation);

            void translate(size_t index, Vec3 translation);

            void rotate(size_t index, Quat rotation);

            void rotateLocal(size_t index, Quat rotation);

            void scale(size_t index, Vec3 scale_factors);

            void setPosition(Entity entity, Vec3 position);

            void setPosition(size_t index, Vec3 position);

            void setOrientation(size_t index, Quat orientation);

            void setParent(size_t index, Entity parent);

            const Vec3 & getPosition(size_t index) const;

            Vec3 getWorldPosition(size_t index) const;

            Vec3 getWorldPosition(Entity e) const;

            const Quat & getOrientation(size_t index) const;

            const Mat4x4 & getWorldTransformation(size_t index) const;

            Mat4x4 const * const getWorldTransformations() const;
        };
    }
}

#endif