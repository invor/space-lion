#include "NameComponentManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        size_t NameComponentManager::addComponent(Entity entity, std::string debug_name)
        {
            auto index = data_.addComponent(
                {
                    entity,
                    std::move(debug_name)
                }
            );

            addIndex(entity.id(), index);

            return index;
        }

        std::string NameComponentManager::getDebugName(size_t index) const
        {
            auto [page_idx, idx_in_page] = data_.getIndices(index);
            return data_(page_idx, idx_in_page).debug_name;
        }

        std::string NameComponentManager::getDebugName(Entity entity) const
        {
            return getDebugName(getIndex(entity));
        }
    }
}