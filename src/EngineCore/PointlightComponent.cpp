#include "PointlightComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        size_t PointlightComponentManager::addComponent(Entity entity, Vec3 light_colour, float lumen, float radius)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    light_colour,
                    lumen,
                    radius
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        Entity PointlightComponentManager::getEntity(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).entity;
        }

        Vec3 PointlightComponentManager::getColour(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).light_colour;
        }

        float PointlightComponentManager::getLumen(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).lumen;
        }

        float PointlightComponentManager::getRadius(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).radius;
        }

        void PointlightComponentManager::setColour(size_t index, Vec3 colour)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).light_colour = colour;
        }

        void PointlightComponentManager::setLumen(size_t index, float lumen)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).lumen = lumen;
        }

        void PointlightComponentManager::setRadius(size_t index, float radius)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).radius = radius;
        }
    }
}