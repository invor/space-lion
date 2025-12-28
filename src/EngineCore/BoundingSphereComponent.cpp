#include "BoundingSphereComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        size_t BoundingSphereComponentManager::addComponent(Entity entity, float radius)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    radius
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        Entity BoundingSphereComponentManager::getEntity(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).entity;
        }

        float BoundingSphereComponentManager::getRadius(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).radius;
        }

        void BoundingSphereComponentManager::setRadius(size_t index, float radius)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).radius = radius;
        }
    }
}