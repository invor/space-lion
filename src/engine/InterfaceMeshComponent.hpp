#ifndef InterfaceMeshComponent_hpp
#define InterfaceMeshComponent_hpp

#include <map>
#include <string>

#include "MTQueue.hpp"
#include "RenderJobs.hpp"
#include "ResourceManager.h"

class InterfaceMeshComponentManager
{	
private:
	/** Data a single component carries. */
	struct Data
	{
		Data(Entity e, std::string material_path, std::string mesh_path, VertexDescriptor vertex_description, GLenum mesh_type)
		: entity(e), material_path(material_path), mesh_path(mesh_path),
			vertex_data(), index_data(), vertex_description(vertex_description), mesh_type(mesh_type) {}

		Entity entity;
		std::string material_path;
		std::string mesh_path;
		std::vector<uint8_t> vertex_data;
		std::vector<uint32_t> index_data;
		VertexDescriptor vertex_description;
		GLenum mesh_type;
	};

	/** Dynamic storage of components data (Since components contains strings, dynamic allocation with a vector is used). */
	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Thread safe queue containing indices of added components that haven't been registerd by the Rendering Pipeline yet. */
	MTQueue<uint> m_added_components_queue;

	MTQueue<uint> m_updated_components_queue;

	std::vector<Data>& getData() { return m_data; }
	MTQueue<uint>& getComponentsQueue()  { return m_added_components_queue; }

	ResourceManager* m_resource_mngr;

	/* Grant Rendering Pipeline access to private members. */
	friend class DeferredRenderingPipeline;

public:
	InterfaceMeshComponentManager(ResourceManager* resource_mngr);
	~InterfaceMeshComponentManager();

	void addComponent(Entity e, std::string material_path, std::string mesh_path);
	
	template<typename VertexContainer, typename IndexContainer>
	void addComponent(Entity e, std::string name, std::string material_path, const VertexContainer& vertices,
						const IndexContainer& indices, const VertexDescriptor& vertex_description, GLenum mesh_type)
	{
		m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

		m_data.push_back(Data(e,material_path,name,vertex_description,mesh_type));

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data.back().vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data.back().vertex_data.data(),vertices.data(),byte_size);

		m_data.back().index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data.back().index_data.begin());

		m_added_components_queue.push(m_data.size()-1);
	}

	template<typename VertexContainer, typename IndexContainer>
	void updateComponent(Entity e, const VertexContainer& vertices, const IndexContainer& indices, const VertexDescriptor& vertex_description, GLenum mesh_type)
	{
		auto search = m_index_map.find(e.id());

		assert( (search != m_index_map.end()) );

		uint idx = search->second;

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data[idx].vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data[idx].vertex_data.data(),vertices.data(),byte_size);

		m_data[idx].index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data[idx].index_data.begin());

		m_updated_components_queue.push(idx);
	}

};

#endif