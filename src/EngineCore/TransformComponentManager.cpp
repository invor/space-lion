#if 0

#include "TransformComponentManager.hpp"

namespace EngineCore
{
    namespace Common
    {
        TransformComponentManager::TransformComponentManager()
            : BaseSingleInstanceComponentManager()
        {}

        TransformComponentManager::TransformComponentManager(size_t size)
            : BaseSingleInstanceComponentManager(), m_data()
        {
            const size_t bytes = size * (sizeof(Entity)
                + sizeof(Mat4x4)
                + 2 * sizeof(Vec3)
                + sizeof(Quat)
                + 3 * sizeof(size_t));
            m_data.buffer = new uint8_t[bytes];

            m_data.used = 0;
            m_data.allocated = size;

            m_data.entity = (Entity*)(m_data.buffer);
            m_data.world_transform = (Mat4x4*)(m_data.entity + size);
            m_data.position = (Vec3*)(m_data.world_transform + size);
            m_data.orientation = (Quat*)(m_data.position + size);
            m_data.scale = (Vec3*)(m_data.orientation + size);
            m_data.parent = (size_t*)(m_data.scale + size);
            m_data.first_child = (size_t*)(m_data.parent + size);
            m_data.next_sibling = (size_t*)(m_data.first_child + size);
        }

        TransformComponentManager::~TransformComponentManager()
        {
            delete[] m_data.buffer;
        }

        void TransformComponentManager::reallocate(size_t size)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(size > m_data.used);

            Data new_data;
            const size_t bytes = size * (sizeof(Entity)
                + sizeof(Mat4x4)
                + 2 * sizeof(Vec3)
                + sizeof(Quat)
                + 3 * sizeof(size_t));
            new_data.buffer = new uint8_t[bytes];

            new_data.used = m_data.used;
            new_data.allocated = size;

            new_data.entity = (Entity*)(new_data.buffer);
            new_data.world_transform = (Mat4x4*)(new_data.entity + size);
            new_data.position = (Vec3*)(new_data.world_transform + size);
            new_data.orientation = (Quat*)(new_data.position + size);
            new_data.scale = (Vec3*)(new_data.orientation + size);
            new_data.parent = (size_t*)(new_data.scale + size);
            new_data.first_child = (size_t*)(new_data.parent + size);
            new_data.next_sibling = (size_t*)(new_data.first_child + size);

            std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
            std::memcpy(new_data.world_transform, m_data.world_transform, m_data.used * sizeof(Mat4x4));
            std::memcpy(new_data.position, m_data.position, m_data.used * sizeof(Vec3));
            std::memcpy(new_data.orientation, m_data.orientation, m_data.used * sizeof(Quat));
            std::memcpy(new_data.scale, m_data.scale, m_data.used * sizeof(Vec3));
            std::memcpy(new_data.parent, m_data.parent, m_data.used * sizeof(uint));
            std::memcpy(new_data.first_child, m_data.first_child, m_data.used * sizeof(uint));
            std::memcpy(new_data.next_sibling, m_data.next_sibling, m_data.used * sizeof(uint));

            delete[] m_data.buffer;

            m_data = new_data;
        }

        size_t TransformComponentManager::addComponent(Entity entity, Vec3 position, Quat orientation, Vec3 scale)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(m_data.used < m_data.allocated);

            // TODO check if entity already has a transform component and issue a warning of some kind

            size_t index = m_data.used;

            addIndex(entity.id(), index);

            m_data.entity[index] = entity;
            m_data.position[index] = position;
            m_data.orientation[index] = orientation;
            m_data.scale[index] = scale;

            m_data.parent[index] = index;
            m_data.first_child[index] = index;
            m_data.next_sibling[index] = index;

            m_data.used++;

            transform(index);

            return index;
        }

        void TransformComponentManager::deleteComonent(Entity entity)
        {
            //TODO
            // std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);
        }

        size_t TransformComponentManager::getComponentCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.used;
        }

        void TransformComponentManager::translate(Entity entity, Vec3 translation)
        {
            auto query = getIndex(entity);

            translate(query, translation);
        }

        void TransformComponentManager::translate(size_t index, Vec3 translation)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.position[index] += translation;

            transform(index);
        }

        void TransformComponentManager::rotate(size_t index, Quat rotation)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            //m_data.orientation[index] *= rotation; // local transform...
            m_data.orientation[index] = glm::normalize(rotation * m_data.orientation[index]);

            transform(index);
        }

        void TransformComponentManager::rotateLocal(size_t index, Quat rotation)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.orientation[index] = glm::normalize(m_data.orientation[index] * rotation);

            transform(index);
        }

        void TransformComponentManager::scale(size_t index, Vec3 scale_factors)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.scale[index] *= scale_factors;

            transform(index);
        }

        void TransformComponentManager::transform(size_t index)
        {
            // std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            Mat4x4 xform = glm::toMat4(m_data.orientation[index]);
            xform[3] = Vec4(m_data.position[index], 1.0);
            xform[0] *= m_data.scale[index].x;
            xform[1] *= m_data.scale[index].y;
            xform[2] *= m_data.scale[index].z;

            if (m_data.parent[index] != index) {
                m_data.world_transform[index] = m_data.world_transform[m_data.parent[index]] * xform;
            }
            else {
                m_data.world_transform[index] = xform;
            }

            // update transforms of all children
            if (m_data.first_child[index] != index)
            {
                size_t child_idx = m_data.first_child[index];
                size_t sibling_idx = m_data.next_sibling[child_idx];

                transform(child_idx);

                while (sibling_idx != child_idx)
                {
                    child_idx = sibling_idx;
                    sibling_idx = m_data.next_sibling[child_idx];

                    transform(child_idx);
                }
            }
        }

        void TransformComponentManager::setPosition(Entity entity, Vec3 position)
        {
            auto query = getIndex(entity);

            setPosition(query, position);
        }

        void TransformComponentManager::setPosition(size_t index, Vec3 position)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.position[index] = position;

            transform(index);
        }

        void TransformComponentManager::setOrientation(size_t index, Quat orientation)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.orientation[index] = orientation;

            transform(index);
        }

        void TransformComponentManager::setScale(size_t index, Vec3 scale)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            m_data.scale[index] = scale;

            transform(index);
        }

        void TransformComponentManager::setParent(size_t index, Entity parent)
        {
            std::unique_lock<std::shared_mutex> lock(m_data_access_mutex);

            auto query = getIndex(parent);


            size_t parent_idx = query;

            m_data.parent[index] = parent_idx;

            if (m_data.first_child[parent_idx] == parent_idx)
            {
                m_data.first_child[parent_idx] = index;
            }
            else
            {
                size_t child_idx = m_data.first_child[parent_idx];

                while (m_data.next_sibling[child_idx] != child_idx)
                {
                    child_idx = m_data.next_sibling[child_idx];
                }

                m_data.next_sibling[child_idx] = index;
            }

            transform(index);
            
        }

        const Vec3 & TransformComponentManager::getPosition(size_t index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.position[index];
        }

        Vec3 TransformComponentManager::getWorldPosition(size_t index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);

            assert(index < m_data.used);

            return Vec3(m_data.world_transform[index] * Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }

        Vec3 TransformComponentManager::getWorldPosition(Entity e) const
        {
            Vec3 retval;

            auto query = getIndex(e);

            retval = getWorldPosition(query);

            return retval;
        }

        const Quat & TransformComponentManager::getOrientation(size_t index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.orientation[index];
        }

        const Mat4x4 & TransformComponentManager::getWorldTransformation(size_t index) const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.world_transform[index];
        }

        Mat4x4 const * const TransformComponentManager::getWorldTransformations() const
        {
            std::shared_lock<std::shared_mutex> lock(m_data_access_mutex);
            return m_data.world_transform;
        }
    }
}

#endif