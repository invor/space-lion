/// <copyright file="BaseMultiInstanceComponentManager.h">
/// Copyright © 2025 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseSingleInstanceComponentManager_hpp
#define BaseSingleInstanceComponentManager_hpp

#include "BaseComponentManager.hpp"
#include "ComponentStorage.hpp"
#include "EntityManager.hpp"
#include "SingleInstanceIndexMap.hpp"

namespace EngineCore
{

    template<typename ComponentDataType>
    class BaseSingleInstanceComponentManager2 : public BaseComponentManager
    {
    protected:
        Utility::SingleInstanceIndexMap             index_map_;
        Utility::ComponentStorage<ComponentDataType, 1000, 1000> data_;

        inline void addIndex(unsigned int entity_id, size_t index)
        {
            index_map_.addIndex(entity_id, index);
        }

    public:
        BaseSingleInstanceComponentManager2() = default;
        ~BaseSingleInstanceComponentManager2() = default;

        BaseSingleInstanceComponentManager2(const BaseSingleInstanceComponentManager2& cpy) = delete;
        BaseSingleInstanceComponentManager2(BaseSingleInstanceComponentManager2&& other) = delete;
        BaseSingleInstanceComponentManager2& operator=(BaseSingleInstanceComponentManager2&& rhs) = delete;
        BaseSingleInstanceComponentManager2& operator=(const BaseSingleInstanceComponentManager2& rhs) = delete;

        inline size_t getIndex(Entity entity) const
        {
            return getIndex(entity.id());
        }

        inline size_t getIndex(unsigned int entity_id) const
        {
            return index_map_.getIndex(entity_id);
        }

        inline size_t addComponent(ComponentDataType component_data)
        {
            auto index = data_.addComponent( std::move(component_data) );

            index_map_.addIndex(entity_id, index);

            auto [page_idx, idx_in_page] = data_.getIndices(index);

            return index;
        }

        inline void deleteComponent(Entity entity)
        {
            auto index = getIndex(entity.id());

            //TODO check for valid index before deletion

            index_map_.deleteIndex(entity);

            data_.deleteComponent(index);
        }

        inline size_t getComponentCount() const
        {
            return data_.getComponentCount();
        }

        inline bool checkComponent(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_.checkComponent(indices.first, indices.second);
        }

        inline ComponentDataType const& getComponent(size_t index) const
        {
            auto indices = data_.getIndices(index);
            return data_(indices.first, indices.second);
        }
    };

}

#endif // !AbstractComponentManager_hpp
