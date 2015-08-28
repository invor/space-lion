#include "TransformComponent.h"

TransformComponentManager::TransformComponentManager() {}

TransformComponentManager::TransformComponentManager(uint size) : m_data()
{
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Mat4x4)
								+ 2*sizeof(Vec3)
								+ sizeof(Quat)
								+ 3*sizeof(uint));
	m_data.buffer = new char[bytes];
	
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
	delete m_data.buffer;
}

void TransformComponentManager::reallocate(uint size)
{
	assert(size > m_data.used);

	Data new_data;
	const uint bytes = size * (sizeof(Entity)
								+ sizeof(Mat4x4)
								+ 2*sizeof(Vec3)
								+ sizeof(Quat)
								+ 3*sizeof(uint));
	new_data.buffer = new char[bytes];
	
	new_data.used = 0;
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

	delete m_data.buffer;

	m_data = new_data;
}

void TransformComponentManager::addComponent(Entity entity)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_data.entity[index] = entity;
	m_data.position[index] = Vec3();
	m_data.orientation[index] = Quat();
	m_data.scale[index] = Vec3();

	m_data.parent[index] = index;
	m_data.first_child[index] = index;
	m_data.next_sibling[index] = index;

	m_data.used++;

	transform(index);
}

void TransformComponentManager::deleteComonent(Entity entity)
{
}

uint TransformComponentManager::getIndex(Entity entity)
{
	auto search = m_index_map.find(entity.id());

	assert( (search == m_index_map.end()) );

	return search->second;
}

void TransformComponentManager::move(uint index, Vec3 translation)
{
	m_data.position[index] += translation;

	transform(index);
}

void TransformComponentManager::rotate(uint index, Quat rotation)
{
	m_data.orientation[index] *= rotation;

	transform(index);
}

void TransformComponentManager::scale(uint index, Vec3 scale_factors)
{
	m_data.scale[index] *= scale_factors;

	transform(index);
}

void TransformComponentManager::transform(uint index)
{
	Mat4x4 parent_transform = m_data.world_transform[m_data.parent[index]];

	Mat4x4 local_translation = glm::translate(Mat4x4(),m_data.position[index]);
	Mat4x4 local_orientation = glm::toMat4(m_data.orientation[index]);
	Mat4x4 local_scaling = glm::scale(Mat4x4(),m_data.scale[index]);

	m_data.world_transform[index] = parent_transform * local_translation;
}

const Mat4x4 TransformComponentManager::getWorldTransformation(uint index)
{
	return m_data.world_transform[index];
}