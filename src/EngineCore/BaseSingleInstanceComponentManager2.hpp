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

    template<typename ComponentDataType>
    class BaseSingleInstanceComponentManager2 : public BaseComponentManager
    {
    protected:
        Utility::SingleInstanceIndexMap             index_map_;
        Utility::ComponentStorage<ComponentDataType, 1000, 1000> data_;

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

    template<typename ComponentDataType>
    inline void BaseSingleInstanceComponentManager2<ComponentDataType>::addIndex(unsigned int entity_id, size_t index)
    {
        index_map_.addIndex(entity_id, index);
    }

    template<typename ComponentDataType>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType>::getIndex(Entity entity) const
    {
        return getIndex(entity.id());
    }

    template<typename ComponentDataType>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType>::getIndex(unsigned int entity_id) const
    {
        return index_map_.getIndex(entity_id);
    }

    template<typename ComponentDataType>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType>::addComponent(ComponentDataType component_data)
    {
        auto index = data_.addComponent(std::move(component_data));

        index_map_.addIndex(component_data.entity.id(), index);

        return index;
    }

    template<typename ComponentDataType>
    inline void BaseSingleInstanceComponentManager2<ComponentDataType>::deleteComponent(Entity entity)
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

    template<typename ComponentDataType>
    inline size_t BaseSingleInstanceComponentManager2<ComponentDataType>::getComponentCount() const
    {
        return data_.getComponentCount();
    }

    template<typename ComponentDataType>
    inline bool BaseSingleInstanceComponentManager2<ComponentDataType>::checkComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_.checkComponent(indices.first, indices.second);
    }

    template<typename ComponentDataType>
    inline ComponentDataType const& BaseSingleInstanceComponentManager2<ComponentDataType>::getComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }

    template<typename ComponentDataType>
    inline ComponentDataType& BaseSingleInstanceComponentManager2<ComponentDataType>::getComponent(size_t index)
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }

}

#endif // !AbstractComponentManager2_hpp
