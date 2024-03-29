#include "BoundingCylinderComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        BoundingCylinderComponentManager::BoundingCylinderComponentManager(uint size)
        {
            const uint bytes = size * (sizeof(Entity)
                + (2 * static_cast<uint>(sizeof(float))));

            m_data.buffer = new uint8_t[bytes];

            m_data.used = 0; // TODO is this correct?
            m_data.allocated = size;

            m_data.entity = (Entity*)(m_data.buffer);
            m_data.radius = (float*) (m_data.entity + size);
            m_data.height = (float*) (m_data.radius + size);
        }

        BoundingCylinderComponentManager::~BoundingCylinderComponentManager()
        {
            // is this thread safe?
            delete[] m_data.buffer;
        }

        void BoundingCylinderComponentManager::reallocate(uint size)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            Data new_data;

            const uint bytes = size * (sizeof(Entity)
                + (2 * static_cast<uint>(sizeof(float))));

            new_data.buffer = new uint8_t[bytes];

            new_data.used = m_data.used;
            new_data.allocated = size;

            new_data.entity = (Entity*)(new_data.buffer);
            new_data.radius = (float*) (new_data.entity + size);
            new_data.height = (float*) (new_data.radius + size);

            std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
            std::memcpy(m_data.radius, new_data.radius, m_data.used * sizeof(float));
            std::memcpy(m_data.height, new_data.height, m_data.used * sizeof(float));

            delete m_data.buffer;

            m_data = new_data;
        }

        void BoundingCylinderComponentManager::addComponent(Entity entity, float radius, float height)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(m_data.used < m_data.allocated);

            uint index = m_data.used;

            addIndex(entity.id(), index);

            m_data.entity[index] = entity;
            m_data.radius[index] = radius;
            m_data.height[index] = height;

            m_data.used++;
        }

        void BoundingCylinderComponentManager::deleteComponent(Entity entity)
        {
            //std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        }

        uint BoundingCylinderComponentManager::getComponentCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.used;
        }

        Entity BoundingCylinderComponentManager::getEntity(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.entity[index];
        }

        float BoundingCylinderComponentManager::getRadius(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.radius[index];
        }

        float BoundingCylinderComponentManager::getHeight(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.height[index];
        }

        void BoundingCylinderComponentManager::setRadius(uint index, float radius)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.radius[index] = radius;
        }

        void BoundingCylinderComponentManager::setHeight(uint index, float height)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.height[index] = height;
        }

        std::vector<Entity> BoundingCylinderComponentManager::getListOfEntities() const
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