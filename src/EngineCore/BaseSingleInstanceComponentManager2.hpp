/// <copyright file="BaseMultiInstanceComponentManager2.hpp">
/// Copyright © 2025 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseSingleInstanceComponentManager2_hpp
#define BaseSingleInstanceComponentManager2_hpp

#include "BaseComponentManager.hpp"
#include "ComponentStorage.hpp"
#include "EntityManager.hpp"
#include "SingleInstanceIndexMap.hpp"

namespace EngineCore
{

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    class BaseSingleInstanceComponentManager2 : public BaseComponentManager
    {
    protected:
        Utility::SingleInstanceIndexMap             index_map_;
        Utility::ComponentStorage<ComponentDataType, PageCount, PageSize> data_;

        void addIndex(unsigned int entity_id, size_t index);

    public:
        BaseSingleInstanceComponentManager2() = default;
        ~BaseSingleInstanceComponentManager2() = default;

        BaseSingleInstanceComponentManager2(const BaseSingleInstanceComponentManager2& cpy) = delete;
        BaseSingleInstanceComponentManager2(BaseSingleInstanceComponentManager2&& other) = delete;
        BaseSingleInstanceComponentManager2& operator=(BaseSingleInstanceComponentManager2&& rhs) = delete;
        BaseSingleInstanceComponentManager2& operator=(const BaseSingleInstanceComponentManager2& rhs) = delete;

        size_t getIndex(Entity entity) const;

        size_t getIndex(unsigned int entity_id) const;

        size_t addComponent(ComponentDataType component_data);

        void deleteComponent(Entity entity);

        size_t getComponentCount() const;

        bool checkComponent(size_t index) const;

        ComponentDataType const& getComponent(size_t index) const;

        ComponentDataType& getComponent(size_t index);
    };

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline void BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::addIndex(unsigned int entity_id, size_t index)
    {
        index_map_.addIndex(entity_id, index);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getIndex(Entity entity) const
    {
        return getIndex(entity.id());
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getIndex(unsigned int entity_id) const
    {
        return index_map_.getIndex(entity_id);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::addComponent(ComponentDataType component_data)
    {
        auto index = data_.addComponent(std::move(component_data));

        index_map_.addIndex(component_data.entity.id(), index);

        return index;
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline void BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::deleteComponent(Entity entity)
    {
        auto index = getIndex(entity.id());

        // check if valid index is given for entity
        // (avoids adding an invalid index to free list of component storage)
        if (index != Utility::SingleInstanceIndexMap::invalidIndex()) {
            index_map_.deleteIndex(entity);

            auto [page_idx, idx_in_page] = data_.getIndices(index);
            data_.deleteComponent(page_idx, idx_in_page);
        }
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponentCount() const
    {
        return data_.getComponentCount();
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline bool BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::checkComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_.checkComponent(indices.first, indices.second);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline ComponentDataType const& BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline ComponentDataType& BaseSingleInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponent(size_t index)
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }

}

#endif // !AbstractComponentManager2_hpp
