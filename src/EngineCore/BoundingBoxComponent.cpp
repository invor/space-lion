#include "BoundingBoxComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        size_t BoundingBoxComponentManager::addComponent(
            Entity entity,
            Vec3   min,
            Vec3   max,
            BBAlignment alignment)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    min,
                    max,
                    alignment
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        BBAlignment BoundingBoxComponentManager::getAlignment(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second).alignment;
        }

        void BoundingBoxComponentManager::setAlignment(size_t index, BBAlignment alignment)
        {
            auto indices = data_.getIndices(index);
            data_(indices.first, indices.second).alignment = alignment;
        }
    }
}