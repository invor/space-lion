/// <copyright file="BaseMultiInstanceComponentManager2.hpp">
/// Copyright © 2025 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseMultiInstanceComponentManager2_hpp
#define BaseMultiInstanceComponentManager2_hpp

#include <assert.h>
#include <shared_mutex>
#include <unordered_map>

#include "BaseComponentManager.hpp"
#include "ComponentStorage.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    class BaseMultiInstanceComponentManager2 : public BaseComponentManager
    {
    protected:

        /// <summary>
        /// Mapping from Entity ID to component index
        /// </summary>
        std::unordered_map<unsigned int, std::vector<size_t>> m_index_map;

        /// <summary>
        /// Mutex for protection of index map add vs read
        /// </summary>
        mutable std::shared_mutex m_index_map_mutex;

        Utility::ComponentStorage<ComponentDataType, PageCount, PageSize> data_;

        void addIndex(unsigned int entity_id, size_t index);

        void rebuildIndexMap();

    public:
        BaseMultiInstanceComponentManager2() = default;
        ~BaseMultiInstanceComponentManager2() = default;

        BaseMultiInstanceComponentManager2(const BaseMultiInstanceComponentManager2& cpy) = delete;
        BaseMultiInstanceComponentManager2(BaseMultiInstanceComponentManager2&& other) = delete;
        BaseMultiInstanceComponentManager2& operator=(BaseMultiInstanceComponentManager2&& rhs) = delete;
        BaseMultiInstanceComponentManager2& operator=(const BaseMultiInstanceComponentManager2& rhs) = delete;

        std::vector<size_t> getIndex(Entity entity) const;

        std::vector<size_t> getIndex(unsigned int entity_id) const;

        size_t addComponent(ComponentDataType component_data);

        void deleteComponent(Entity entity, size_t sub_index);

        size_t getComponentCount() const;

        bool checkComponent(size_t index) const;

        ComponentDataType const& getComponent(size_t index) const;

        ComponentDataType& getComponent(size_t index);
    };

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline void BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::addIndex(unsigned int entity_id, size_t index)
    {
        std::unique_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

        auto query = m_index_map.find(entity_id);

        if (query != m_index_map.end())
        {
            query->second.push_back(index);
        }
        else
        {
            m_index_map.insert({ entity_id, {index} });
        }
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline void BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::rebuildIndexMap()
    {
        std::unique_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

        m_index_map.clear();

        for (size_t idx = 0; idx < data_.getComponentCount(); ++idx) {
            auto [page_idx, idx_in_page] = data_.getIndices(idx);
            auto query = m_index_map.find(data_(page_idx, idx_in_page).entity.id());

            if (query != m_index_map.end())
            {
                query->second.push_back(idx);
            }
            else
            {
                m_index_map.insert({ data(page_idx, idx_in_page).entity.id(), {idx} });
            }
        }
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline std::vector<size_t> BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getIndex(Entity entity) const
    {
        return getIndex(entity.id());
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline std::vector<size_t> BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getIndex(unsigned int entity_id) const
    {
        std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

        //std::vector<size_t> retval;

        auto query = m_index_map.find(entity_id);

        if (query != m_index_map.end())
        {
            return query->second;
        }

        //return std::move(retval);
        return std::vector<size_t>();
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::addComponent(ComponentDataType component_data)
    {
        auto index = data_.addComponent(std::move(component_data));

        addIndex(component_data.entity.id(), index);

        auto [page_idx, idx_in_page] = data_.getIndices(index);

        return index;
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline void BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::deleteComponent(Entity entity, size_t sub_index)
    {
        std::vector<size_t> indices = getIndex(entity);

        if (indices.size() > sub_index)
        {
            auto [page_idx, idx_in_page] = data_.getIndices(indices[sub_index]);
            data_.deleteComponent(page_idx, idx_in_page);

            rebuildIndexMap();
        }
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline size_t BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponentCount() const
    {
        return data_.getComponentCount();
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline bool BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::checkComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_.checkComponent(indices.first, indices.second);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline ComponentDataType const& BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponent(size_t index) const
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }

    template<typename ComponentDataType, size_t PageCount, size_t PageSize>
    inline ComponentDataType& BaseMultiInstanceComponentManager2<ComponentDataType, PageCount, PageSize>::getComponent(size_t index)
    {
        auto indices = data_.getIndices(index);
        return data_(indices.first, indices.second);
    }
}

#endif // !BaseMultiInstanceComponentManager2_hpp
