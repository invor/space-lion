#include "PointlightComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        PointlightComponentManager::PointlightComponentManager(uint size)
        {
            const uint bytes = size * (sizeof(Entity)
                + sizeof(Vec3)
                + (2 * static_cast<uint>(sizeof(float))));
            m_data.buffer = new uint8_t[bytes];

            m_data.used = 0;
            m_data.allocated = size;

            m_data.entity = (Entity*)(m_data.buffer);
            m_data.light_colour = (Vec3*)(m_data.entity + size);
            m_data.lumen = (float*)(m_data.light_colour + size);
            m_data.radius = (float*)(m_data.lumen + size);

        }

        PointlightComponentManager::~PointlightComponentManager()
        {
            delete[] m_data.buffer;
        }

        void PointlightComponentManager::reallocate(uint size)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            Data new_data;

            const uint bytes = size * (sizeof(Entity)
                + sizeof(Vec3)
                + (2 * static_cast<uint>(sizeof(float))));
            new_data.buffer = new uint8_t[bytes];

            new_data.used = m_data.used;
            new_data.allocated = size;

            new_data.entity = (Entity*)(new_data.buffer);
            new_data.light_colour = (Vec3*)(new_data.entity + size);
            new_data.lumen = (float*)(new_data.light_colour + size);
            new_data.radius = (float*)(new_data.lumen + size);

            std::memcpy(m_data.entity, new_data.entity, m_data.used * sizeof(Entity));
            std::memcpy(m_data.light_colour, new_data.light_colour, m_data.used * sizeof(Vec3));
            std::memcpy(m_data.lumen, new_data.lumen, m_data.used * sizeof(float));
            std::memcpy(m_data.radius, new_data.radius, m_data.used * sizeof(float));

            delete m_data.buffer;

            m_data = new_data;
        }

        void PointlightComponentManager::addComponent(Entity entity, Vec3 light_colour, float lumen, float radius)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(m_data.used < m_data.allocated);

            uint index = m_data.used;

            addIndex(entity.id(),index);

            m_data.entity[index] = entity;
            m_data.light_colour[index] = light_colour;
            m_data.lumen[index] = lumen;
            m_data.radius[index] = radius;

            m_data.used++;
        }

        void PointlightComponentManager::deleteComponent(Entity entity)
        {
            // std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        }

        uint PointlightComponentManager::getComponentCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.used;
        }

        Entity PointlightComponentManager::getEntity(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.entity[index];
        }

        Vec3 PointlightComponentManager::getColour(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.light_colour[index];
        }

        float PointlightComponentManager::getLumen(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.lumen[index];
        }

        float PointlightComponentManager::getRadius(uint index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.radius[index];
        }

        void PointlightComponentManager::setColour(uint index, Vec3 colour)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.light_colour[index] = colour;
        }

        void PointlightComponentManager::setLumen(uint index, float lumen)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.lumen[index] = lumen;
        }

        void PointlightComponentManager::setRadius(uint index, float radius)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
            m_data.radius[index] = radius;
        }

        std::vector<Entity> PointlightComponentManager::getListOfEntities() const
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