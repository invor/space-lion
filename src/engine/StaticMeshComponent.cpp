#include "StaticMeshComponent.hpp"

StaticMeshComponentManager::StaticMeshComponentManager(ResourceManager* resource_mngr)
	: m_resource_mngr(resource_mngr)

{
	// nothing to take care of because memory allocation is dynamically handled by std::vector and addComponents methods
}

StaticMeshComponentManager::~StaticMeshComponentManager()
{
	// nothing to take care of because memory allocation is dynamically handled by std::vector
}

void StaticMeshComponentManager::addComponent(Entity e, std::string material_path, std::string mesh_path, bool cast_shadow)
{
	m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

	m_data.push_back(Data(e,nullptr,nullptr,material_path,mesh_path,VertexDescriptor(0,{}),GL_TRIANGLES,cast_shadow));

	m_resource_mngr->loadFbxGeometry(mesh_path,m_data.back().vertex_data,m_data.back().index_data,m_data.back().vertex_description);

	m_added_components_queue.push(m_data.size()-1);
}

void StaticMeshComponentManager::addComponent(Entity e, Mesh* mesh, Material* material, bool cast_shadow)
{
	m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

	//TODO not that it matters but GL_TRIANGLES could be wrong in some cases...so better query the type from mesh
	m_data.push_back(Data(e,material,mesh,material->getName(),mesh->getName(),VertexDescriptor(0,{}),GL_TRIANGLES,cast_shadow));

	m_added_components_queue.push(m_data.size()-1);
}