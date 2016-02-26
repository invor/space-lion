#ifndef StaticMeshComponent_hpp
#define StaticMeshComponent_hpp

#include <map>
#include <string>

#include "MTQueue.hpp"
#include "RenderJobs.hpp"
#include "ResourceManager.h"

class StaticMeshComponentManager
{	
private:
	/** Data a single component carries. */
	struct Data
	{
		Data(Entity e, std::string material_path, std::string mesh_path, VertexDescriptor vertex_description, GLenum mesh_type, bool cast_shadow)
		: entity(e), material_path(material_path), mesh_path(mesh_path),
			vertex_data(), index_data(), vertex_description(vertex_description), mesh_type(mesh_type), cast_shadow(cast_shadow) {}

		Entity entity;
		std::string material_path;
		std::string mesh_path;
		std::vector<uint8_t> vertex_data;
		std::vector<uint32_t> index_data;
		VertexDescriptor vertex_description;
		GLenum mesh_type;
		bool cast_shadow;
	};

	/** Dynamic storage of components data (Since components contains strings, dynamic allocation with a vector is used). */
	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Thread safe queue containing indices of added components that haven't been registerd by the Rendering Pipeline yet. */
	MTQueue<uint> m_added_components_queue;

	std::vector<Data>& getData() { return m_data; }
	MTQueue<uint>& getComponentsQueue()  { return m_added_components_queue; }

	ResourceManager* m_resource_mngr;

	/* Grant Rendering Pipeline access to private members. */
	friend class DeferredRenderingPipeline;

public:
	StaticMeshComponentManager(ResourceManager* resource_mngr);
	~StaticMeshComponentManager();

	void addComponent(Entity e, std::string material_path, std::string mesh_path, bool cast_shadow);
	
	template<typename VertexContainer, typename IndexContainer>
	void addComponent(Entity e, std::string name, std::string material_path, const VertexContainer& vertices,
						const IndexContainer& indices, const VertexDescriptor& vertex_description, GLenum mesh_type, bool cast_shadow)
	{
		m_index_map.insert(std::pair<uint,uint>(e.id(),m_data.size()));

		m_data.push_back(Data(e,material_path,name,vertex_description,mesh_type,cast_shadow));

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		// TODO find a nice c++ way to do this
		m_data.back().vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data.back().vertex_data.data(),vertices.data(),byte_size);

		m_data.back().index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data.back().index_data.begin());

		m_added_components_queue.push(m_data.size()-1);
	}

};

#endif