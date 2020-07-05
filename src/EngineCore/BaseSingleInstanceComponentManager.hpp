/// <copyright file="BaseMultiInstanceComponentManager.h">
/// Copyright © 2020 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef BaseSingleInstanceComponentManager_hpp
#define BaseSingleInstanceComponentManager_hpp

#include <assert.h>
#include <shared_mutex>
#include <unordered_map>

#include "BaseComponentManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{

    class BaseSingleInstanceComponentManager : public BaseComponentManager
    {
    protected:

        /// <summary>
        /// Mapping from Entity ID to component index
        /// </summary>
        std::unordered_map<unsigned int, size_t> m_index_map;

        /// <summary>
        /// Mutex for protection of index map add vs read
        /// </summary>
        mutable std::shared_mutex m_index_map_mutex;

        inline void addIndex(unsigned int entity_id, size_t index)
        {
            std::unique_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

            //auto query = m_index_map.find(entity_id);
            //
            //if (query != m_index_map.end())
            //{
            //    query->second.push_back(index);
            //}
            //else
            //{
            //    m_index_map.insert({ entity_id, {index} });
            //}

            m_index_map.insert({ entity_id, index });
        }

    public:
        BaseSingleInstanceComponentManager() = default;
        ~BaseSingleInstanceComponentManager() = default;

        BaseSingleInstanceComponentManager(const BaseSingleInstanceComponentManager& cpy) = delete;
        BaseSingleInstanceComponentManager(BaseSingleInstanceComponentManager&& other) = delete;
        BaseSingleInstanceComponentManager& operator=(BaseSingleInstanceComponentManager&& rhs) = delete;
        BaseSingleInstanceComponentManager& operator=(const BaseSingleInstanceComponentManager& rhs) = delete;

        inline size_t getIndex(Entity entity) const
        {
            return getIndex(entity.id());
        }

        inline size_t getIndex(unsigned int entity_id) const
        {
            std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

            size_t retval = (std::numeric_limits<size_t>::max)();

            auto query = m_index_map.find(entity_id);

            if (query != m_index_map.end())
            {
                retval = query->second;
            }

            return retval;
        }
    };

}

#endif // !AbstractComponentManager_hpp
