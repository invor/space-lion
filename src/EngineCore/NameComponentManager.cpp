#include "NameComponentManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        void NameComponentManager::addComponent(Entity entity, std::string const& debug_name)
        {
            std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

            uint idx = static_cast<uint>(m_data.size());

            addIndex(entity.id(), idx);

            m_data.push_back(Data(entity, debug_name));
        }

        void NameComponentManager::addComponent(Entity entity, std::string && debug_name)
        {
            std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

            uint idx = static_cast<uint>(m_data.size());

            addIndex(entity.id(), idx);

            m_data.push_back(Data(entity, debug_name));
        }

        std::string NameComponentManager::getDebugName(Entity entity) const
        {
            auto query = getIndex(entity);

            std::string retval;

            if (!query.empty())
                retval = m_data[query.front()].debug_name;

            return retval;
        }

        std::string NameComponentManager::getDebugName(size_t index) const
        {
            return m_data[index].debug_name;
        }
    }
}