#ifndef DebugNameComponent_hpp
#define DebugNameComponent_hpp

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "BaseComponentManager.hpp"
#include "EntityManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        class NameComponentManager : public BaseComponentManager
        {
        private:
            struct Data
            {
                Data(Entity entity, std::string const& debug_name)
                    : entity(entity), debug_name(debug_name) {}
                Data(Entity entity, std::string && debug_name)
                    : entity(entity), debug_name(std::move(debug_name)) {}

                Entity entity;
                std::string debug_name;
            };

            std::vector<Data> m_data;
            std::mutex m_dataAccess_mutex;

        public:
            void addComponent(Entity entity, std::string const& debug_name);
            void addComponent(Entity entity, std::string && debug_name);

            std::string getDebugName(Entity entity) const;
            std::string getDebugName(size_t index) const;
        };
    }
}

#endif // !DebugNameComponent_hpp
