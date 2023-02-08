#include "BoundingSphereComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        BoundingSphereComponentManager::BoundingSphereComponentManager(uint size)
        {
            const uint bytes = size * (sizeof(Entity)
                + static_cast<uint>(sizeof(float)));

            m_data.buffer = new uint8_t[bytes];

            m_data.used = 0;
            m_data.allocated = size;

            m_data.entity = (Entity*)(m_data.buffer);
            m_data.radius = (float*) (m_data.entity + size);
        }

        BoundingSphereComponentManager::~BoundingSphereComponentManager()
        {
            delete[] m_data.buffer;
        }

        void BoundingSphereComponentManager::reallocate(uint size)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            Data new_data;

            const uint bytes = size * (sizeof(Entity)
                + static_cast<uint>(sizeof(float)));

            new_data.buffer = new uint8_t[bytes];

            new_data.used = m_data.used;
            new_data.allocated = size;

            new_data.entity = (Entity*)(new_data.buffer);
            new_data.radius = (float*) (new_data.entity + size);

            std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
            std::memcpy(m_data.radius, new_data.radius, m_data.used * sizeof(float));

            delete m_data.buffer;

            m_data = new_data;
        }

        void BoundingSphereComponentManager::addComponent(Entity entity, float radius)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(m_data.used < m_data.allocated);

            uint index = m_data.used;

            addIndex(entity.id(), index);

            m_data.entity[index] = entity;
            m_data.radius[index] = radius;

            m_data.used++;
        }

        void BoundingSphereComponentManager::deleteComponent(Entity entity)
        {
            // std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        }

        uint BoundingSphereComponentManager::getComponentCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.used;
        }

        Entity BoundingSphereComponentManager::getEntity(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.entity[index];
        }

        float BoundingSphereComponentManager::getRadius(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.radius[index];
        }

        void BoundingSphereComponentManager::setRadius(uint index, float radius)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.radius[index] = radius;
        }

        std::vector<Entity> BoundingSphereComponentManager::getListOfEntities() const
        {
            std::vector<Entity> rtn;

            {
                std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);

                for (uint i = 0; i < m_data.used; i++)
                {
                    rtn.push_back(m_data.entity[i]);
                }
            }

            return rtn;
        }

    }
}