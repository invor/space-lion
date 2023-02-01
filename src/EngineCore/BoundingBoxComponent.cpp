#include "BoundingBoxComponent.hpp"

namespace EngineCore
{
    namespace Graphics
    {

        BoundingBoxComponentManager::BoundingBoxComponentManager(uint size)
        {
            const uint bytes = size * (sizeof(Entity)
                + (3 * static_cast<uint>(sizeof(float)))
                + sizeof(BBAlignment));

            m_data.buffer = new uint8_t[bytes];

            m_data.used = 0;
            m_data.allocated = size;

            m_data.entity    = (Entity*)     (m_data.buffer);
            m_data.width     = (float*)      (m_data.entity + size);
            m_data.height    = (float*)      (m_data.width  + size);
            m_data.depth     = (float*)      (m_data.height + size);
            m_data.alignment = (BBAlignment*)(m_data.depth  + size);

        }

        BoundingBoxComponentManager::~BoundingBoxComponentManager()
        {
            delete[] m_data.buffer;
        }

        void BoundingBoxComponentManager::reallocate(uint size)
        {
            Data new_data;

            const uint bytes = size * (sizeof(Entity)
                + (3 * static_cast<uint>(sizeof(float)))
                + sizeof(BBAlignment));

            new_data.buffer = new uint8_t[bytes];

            new_data.used = m_data.used;
            new_data.allocated = size;

            new_data.entity    = (Entity*)     (new_data.buffer);
            new_data.width     = (float*)      (new_data.entity + size);
            new_data.height    = (float*)      (new_data.width  + size);
            new_data.depth     = (float*)      (new_data.height + size);
            new_data.alignment = (BBAlignment*)(new_data.depth  + size);

            std::memcpy(m_data.entity,    new_data.entity,    m_data.used * sizeof(Entity));
            std::memcpy(m_data.width,     new_data.width,     m_data.used * sizeof(float));
            std::memcpy(m_data.height,    new_data.height,    m_data.used * sizeof(float));
            std::memcpy(m_data.depth,     new_data.depth,     m_data.used * sizeof(float));
            std::memcpy(m_data.alignment, new_data.alignment, m_data.used * sizeof(BBAlignment));

            delete m_data.buffer;

            m_data = new_data;
        }

        void BoundingBoxComponentManager::addComponent(Entity entity, float width, float height, float depth, BBAlignment alignment)
        {
            assert(m_data.used < m_data.allocated);

            uint index = m_data.used;

            addIndex(entity.id(), index);

            m_data.entity[index]    = entity;
            m_data.width[index]     = width;
            m_data.height[index]    = height;
            m_data.depth[index]     = depth;
            m_data.alignment[index] = alignment;

            m_data.used++;
        }

        void BoundingBoxComponentManager::deleteComponent(Entity entity)
        {

        }

        uint BoundingBoxComponentManager::getComponentCount() const
        {
            return m_data.used;
        }

        Entity BoundingBoxComponentManager::getEntity(uint index) const
        {
            return m_data.entity[index];
        }

        float BoundingBoxComponentManager::getWidth(uint index) const
        {
            return m_data.width[index];
        }

        float BoundingBoxComponentManager::getHeight(uint index) const
        {
            return m_data.height[index];
        }

        float BoundingBoxComponentManager::getDepth(uint index) const
        {
            return m_data.depth[index];
        }

        BoundingBoxComponentManager::BBAlignment BoundingBoxComponentManager::getAlignment(uint index) const
        {
            return m_data.alignment[index];
        }

        void BoundingBoxComponentManager::setWidth(uint index, float width)
        {
            m_data.width[index] = width;
        }

        void BoundingBoxComponentManager::setHeight(uint index, float height)
        {
            m_data.height[index] = height;
        }

        void BoundingBoxComponentManager::setDepth(uint index, float depth)
        {
            m_data.depth[index] = depth;
        }

        void BoundingBoxComponentManager::setAlignment(uint index, BBAlignment alignment)
        {
            m_data.alignment[index] = alignment;
        }

        std::vector<Entity> BoundingBoxComponentManager::getListOfEntities() const
        {
            std::vector<Entity> rtn;

            for (uint i = 0; i < m_data.used; i++)
            {
                rtn.push_back(m_data.entity[i]);
            }

            return rtn;
        }

    }
}