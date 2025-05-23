#ifndef DebugNameComponent_hpp
#define DebugNameComponent_hpp

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseSingleInstanceComponentManager2.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        struct NameComponentData
        {
            NameComponentData()
                : entity(), debug_name("") {
            }

            NameComponentData(Entity entity, std::string debug_name)
                : entity(entity), debug_name(std::move(debug_name)) {
            }

            Entity entity;
            std::string debug_name;
        };

        class NameComponentManager : public BaseSingleInstanceComponentManager2<NameComponentData,1000,1000>
        {
        public:
            size_t addComponent(Entity entity, std::string debug_name);

            std::string getDebugName(size_t index) const;

            std::string getDebugName(Entity entity) const;
        };
    }
}

#endif // !DebugNameComponent_hpp
