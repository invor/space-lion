#include "DebugNameComponentManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        void NameComponentManager::addComponent(Entity entity, std::string const& debug_name)
        {
            //std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

            uint idx = static_cast<uint>(m_data.size());

            m_index_map.insert(std::pair<uint, uint>(entity.id(), idx));

            m_data.push_back(Data(entity, debug_name));
        }

        void NameComponentManager::addComponent(Entity entity, std::string && debug_name)
        {
            //std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

            uint idx = static_cast<uint>(m_data.size());

            m_index_map.insert(std::pair<uint, uint>(entity.id(), idx));

            m_data.push_back(Data(entity, debug_name));
        }

        std::pair<bool, uint> NameComponentManager::getIndex(Entity entity) const
        {
            auto search = m_index_map.find(entity.id());

            std::pair<bool, uint> rtn(false, -1);

            if (search != m_index_map.end())
            {
                rtn.first = true;
                rtn.second = search->second;
            }

            return rtn;
        }

        std::string NameComponentManager::getDebugName(Entity entity) const
        {
            auto idx_pair = getIndex(entity);

            std::string rtn;

            if (idx_pair.first)
                rtn = m_data[idx_pair.second].debug_name;

            return rtn;
        }

        std::string NameComponentManager::getDebugName(uint index) const
        {
            return m_data[index].debug_name;
        }
    }
}