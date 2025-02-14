#include "BoundingCylinderComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        size_t BoundingCylinderComponentManager::addComponent(Entity entity, float radius, float height)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    radius,
                    height
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        Entity BoundingCylinderComponentManager::getEntity(uint index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).entity;
        }

        float BoundingCylinderComponentManager::getRadius(uint index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).radius;
        }

        float BoundingCylinderComponentManager::getHeight(uint index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).height;
        }

        void BoundingCylinderComponentManager::setRadius(uint index, float radius)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).radius = radius;
        }

        void BoundingCylinderComponentManager::setHeight(uint index, float height)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).height = height;
        }
    }
}