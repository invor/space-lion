#include "SunlightComponentManager.hpp"

#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Graphics
    {
        size_t SunlightComponentManager::addComponent(Entity entity, Vec3 light_colour, float lumen, float star_radius)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    light_colour,
                    lumen,
                    star_radius
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        void SunlightComponentManager::setColour(size_t index, Vec3 colour)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).light_colour = colour;
        }

        void SunlightComponentManager::setLumen(size_t index, float lumen)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).lumen = lumen;
        }

        void SunlightComponentManager::setStarRadius(size_t index, float radius)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_(page_idx, idx_in_page).star_radius = radius;
        }

        Entity SunlightComponentManager::getEntity(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).entity;
        }

        Vec3 SunlightComponentManager::getColour(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).light_colour;
        }

        float SunlightComponentManager::getLumen(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).lumen;
        }

        float SunlightComponentManager::getStarRadius(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).star_radius;
        }

    }
}