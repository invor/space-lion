#include "TransformComponentManager.hpp"

namespace EngineCore
{
	namespace Common
	{
		TransformComponentManager::TransformComponentManager() {}

		TransformComponentManager::TransformComponentManager(uint size) : m_data()
		{
			const uint bytes = size * (sizeof(Entity)
				+ sizeof(Mat4x4)
				+ 2 * sizeof(Vec3)
				+ sizeof(Quat)
				+ 3 * sizeof(uint));
			m_data.buffer = new uint8_t[bytes];

			m_data.used = 0;
			m_data.allocated = size;

			m_data.entity = (Entity*)(m_data.buffer);
			m_data.world_transform = (Mat4x4*)(m_data.entity + size);
			m_data.position = (Vec3*)(m_data.world_transform + size);
			m_data.orientation = (Quat*)(m_data.position + size);
			m_data.scale = (Vec3*)(m_data.orientation + size);
			m_data.parent = (uint*)(m_data.scale + size);
			m_data.first_child = (uint*)(m_data.parent + size);
			m_data.next_sibling = (uint*)(m_data.first_child + size);
		}

		TransformComponentManager::~TransformComponentManager()
		{
			delete[] m_data.buffer;
		}

		void TransformComponentManager::reallocate(uint size)
		{
			//TODO data protection mutex

			assert(size > m_data.used);

			Data new_data;
			const uint bytes = size * (sizeof(Entity)
				+ sizeof(Mat4x4)
				+ 2 * sizeof(Vec3)
				+ sizeof(Quat)
				+ 3 * sizeof(uint));
			new_data.buffer = new uint8_t[bytes];

			new_data.used = m_data.used;
			new_data.allocated = size;

			new_data.entity = (Entity*)(new_data.buffer);
			new_data.world_transform = (Mat4x4*)(new_data.entity + size);
			new_data.position = (Vec3*)(new_data.world_transform + size);
			new_data.orientation = (Quat*)(new_data.position + size);
			new_data.scale = (Vec3*)(new_data.orientation + size);
			new_data.parent = (uint*)(new_data.scale + size);
			new_data.first_child = (uint*)(new_data.parent + size);
			new_data.next_sibling = (uint*)(new_data.first_child + size);

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

		void TransformComponentManager::addComponent(Entity entity, Vec3 position, Quat orientation, Vec3 scale)
		{
			std::unique_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

			assert(m_data.used < m_data.allocated);

			uint index = m_data.used;

			m_index_map.insert({ entity.id(),index });

			m_data.entity[index] = entity;
			m_data.position[index] = position;
			m_data.orientation[index] = orientation;
			m_data.scale[index] = scale;

			m_data.parent[index] = index;
			m_data.first_child[index] = index;
			m_data.next_sibling[index] = index;

			m_data.used++;

			transform(index);
		}

		void TransformComponentManager::deleteComonent(Entity entity)
		{
			//TODO
		}

		bool TransformComponentManager::checkComponent(uint entity_id) const
		{
			std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

			auto search = m_index_map.find(entity_id);

			if (search == m_index_map.end())
				return false;
			else
				return true;
		}

		uint TransformComponentManager::getComponentCount() const
		{
			return m_data.used;
		}

		uint TransformComponentManager::getIndex(Entity entity) const
		{
			std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

			auto search = m_index_map.find(entity.id());

			assert((search != m_index_map.end()));

			return search->second;
		}

		uint TransformComponentManager::getIndex(uint entity_id) const
		{
			std::shared_lock<std::shared_mutex> index_map_lock(m_index_map_mutex);

			auto search = m_index_map.find(entity_id);

			assert((search != m_index_map.end()));

			return search->second;
		}

		void TransformComponentManager::translate(Entity entity, Vec3 translation)
		{
			translate(getIndex(entity), translation);
		}

		void TransformComponentManager::translate(uint index, Vec3 translation)
		{
			m_data.position[index] += translation;

			transform(index);
		}

		void TransformComponentManager::rotate(uint index, Quat rotation)
		{
			//m_data.orientation[index] *= rotation; // local transform...
			m_data.orientation[index] = glm::normalize(rotation * m_data.orientation[index]);

			transform(index);
		}

		void TransformComponentManager::rotateLocal(uint index, Quat rotation)
		{
			m_data.orientation[index] = glm::normalize(m_data.orientation[index] * rotation);

			transform(index);
		}

		void TransformComponentManager::scale(uint index, Vec3 scale_factors)
		{
			m_data.scale[index] *= scale_factors;

			transform(index);
		}

		void TransformComponentManager::transform(uint index)
		{
			Mat4x4 parent_transform(1.0);

			if (m_data.parent[index] != index)
				parent_transform = m_data.world_transform[m_data.parent[index]];


			Mat4x4 local_translation = glm::translate(Mat4x4(), m_data.position[index]);
			Mat4x4 local_orientation = glm::toMat4(m_data.orientation[index]);
			Mat4x4 local_scaling = glm::scale(Mat4x4(), m_data.scale[index]);

			m_data.world_transform[index] = parent_transform * local_translation * local_orientation * local_scaling;

			// update transforms of all children
			if (m_data.first_child[index] != index)
			{
				uint child_idx = m_data.first_child[index];
				uint sibling_idx = m_data.next_sibling[child_idx];

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
			setPosition(getIndex(entity), position);
		}

		void TransformComponentManager::setPosition(uint index, Vec3 position)
		{
			m_data.position[index] = position;

			transform(index);
		}

		void TransformComponentManager::setOrientation(uint index, Quat orientation)
		{
			m_data.orientation[index] = orientation;

			transform(index);
		}

		void TransformComponentManager::setParent(uint index, Entity parent)
		{
			uint parent_idx = getIndex(parent);

			m_data.parent[index] = parent_idx;

			if (m_data.first_child[parent_idx] == parent_idx)
			{
				m_data.first_child[parent_idx] = index;
			}
			else
			{
				uint child_idx = m_data.first_child[parent_idx];

				while (m_data.next_sibling[child_idx] != child_idx)
				{
					child_idx = m_data.next_sibling[child_idx];
				}

				m_data.next_sibling[child_idx] = index;
			}

			transform(index);
		}

		const Vec3 & TransformComponentManager::getPosition(uint index) const
		{
			return m_data.position[index];
		}

		Vec3 TransformComponentManager::getWorldPosition(uint index) const
		{
			assert(index < m_data.used);

			return Vec3(m_data.world_transform[index] * Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		Vec3 TransformComponentManager::getWorldPosition(Entity e) const
		{
			uint idx = getIndex(e);

			return getWorldPosition(idx);
		}

		const Quat & TransformComponentManager::getOrientation(uint index) const
		{
			return m_data.orientation[index];
		}

		const Mat4x4 & TransformComponentManager::getWorldTransformation(uint index) const
		{
			return m_data.world_transform[index];
		}

		Mat4x4 const * const TransformComponentManager::getWorldTransformations() const
		{
			return m_data.world_transform;
		}
	}
}