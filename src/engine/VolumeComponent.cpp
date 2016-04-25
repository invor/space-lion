#include "VolumeComponent.hpp"

VolumeComponentManager::VolumeComponentManager()
{
}

VolumeComponentManager::~VolumeComponentManager()
{
}

uint VolumeComponentManager::getIndex(Entity e)
{
	auto search = m_index_map.find(e.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

void VolumeComponentManager::addComponent(Entity e, std::string& volume_path, std::string& boundingGeometry_path)
{
}

void VolumeComponentManager::addComponent(Entity e, std::shared_ptr<Texture3D> volume, std::shared_ptr<Mesh> boundingGeometry)
{
	uint idx = m_data.size();

	m_data.push_back(Data(e,volume,boundingGeometry));
	m_index_map.insert(std::pair<uint,uint>(e.id(),idx));
	m_update_queue.push(idx);
}