/// <copyright file="BaseComponentManager.h">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseComponentManager_hpp
#define BaseComponentManager_hpp

#include <assert.h>
#include <shared_mutex>
#include <unordered_map>

#include "EntityManager.hpp"

namespace EngineCore
{

    class BaseComponentManager
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

        inline void addIndex(unsigned int entity_id, size_t index)
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

    public:
        BaseComponentManager() = default;
        ~BaseComponentManager() = default;

        BaseComponentManager(const BaseComponentManager& cpy) = delete;
        BaseComponentManager(BaseComponentManager&& other) = delete;
        BaseComponentManager& operator=(BaseComponentManager&& rhs) = delete;
        BaseComponentManager& operator=(const BaseComponentManager& rhs) = delete;

        inline std::vector<size_t> getIndex(Entity entity) const
        {
            return getIndex(entity.id());
        }

        inline std::vector<size_t> getIndex(unsigned int entity_id) const
        {
            std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

            std::vector<size_t> retval;

            auto query = m_index_map.find(entity_id);

            if (query != m_index_map.end())
            {
                return query->second;
            }

            //return std::move(retval);
            return std::vector<size_t>();
        }
    };

}

#endif // !AbstractComponentManager_hpp
