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

#include "SingleInstanceIndexMap.hpp"

namespace EngineCore
{

    class BaseSingleInstanceComponentManager : public BaseComponentManager
    {
    protected:
        Utility::SingleInstanceIndexMap index_map_;

        inline void addIndex(unsigned int entity_id, size_t index)
        {
            index_map_.addIndex(entity_id, index);
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
            return index_map_.getIndex(entity_id);
        }
    };

}

#endif // !AbstractComponentManager_hpp
