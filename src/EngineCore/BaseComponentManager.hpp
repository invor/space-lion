/// <copyright file="BaseComponentManager.h">
/// Copyright © 2020 Michael Becher. Alle Rechte vorbehalten.
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
    public:
        BaseComponentManager() = default;
        ~BaseComponentManager() = default;

        BaseComponentManager(const BaseComponentManager& cpy) = delete;
        BaseComponentManager(BaseComponentManager&& other) = delete;
        BaseComponentManager& operator=(BaseComponentManager&& rhs) = delete;
        BaseComponentManager& operator=(const BaseComponentManager& rhs) = delete;
    };

}

#endif // !AbstractComponentManager_hpp
