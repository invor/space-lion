#include "InterfaceMeshComponent.hpp"

InterfaceMeshComponentManager::InterfaceMeshComponentManager()
{
	// nothing to take care of because memory allocation is dynamically handled by std::vector and addComponents methods
}

InterfaceMeshComponentManager::~InterfaceMeshComponentManager()
{
	// nothing to take care of because memory allocation is dynamically handled by std::vector
}

void InterfaceMeshComponentManager::addComponent(Entity e, std::string material_path, std::string mesh_path)
{
	m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

	m_data.push_back(Data(e,material_path,mesh_path,VertexDescriptor(0,{}),GL_TRIANGLES));

	GCoreComponents::resourceManager().loadFbxGeometry(mesh_path,m_data.back().vertex_data,m_data.back().index_data,m_data.back().vertex_description);

	m_added_components_queue.push(m_data.size()-1);
}