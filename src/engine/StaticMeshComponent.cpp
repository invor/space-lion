#include "StaticMeshComponent.hpp"

StaticMeshComponentManager::StaticMeshComponentManager()
{
	// nothing to take care of because memory allocation is dynamically handled by std::vector
}

StaticMeshComponentManager::~StaticMeshComponentManager()
{
	// nothing to take care of because memory allocation is dynamically handled by std::vector
}

void StaticMeshComponentManager::addComponent(Entity e, std::string material_path, std::string mesh_path, bool cast_shadow)
{
	m_added_components_queue.push(m_data.size());

	m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

	m_data.push_back(Data(e,material_path,mesh_path,cast_shadow));
}