#include "GlobalCoreComponents.hpp"

#ifndef StaticMeshComponent_hpp
#define StaticMeshComponent_hpp

#include <unordered_map>
#include <string>

#include "GlobalEngineCore.hpp"
#include "ResourceManager.hpp"
#include "DeferredRenderingPipeline.hpp"

#include "ResourceLoading.hpp"

#include "TransformComponent.hpp"

#include "MTQueue.hpp"
#include "RenderJobs.hpp"

struct Frame;

namespace MeshComponent
{
	void drawStaticMeshes(const Frame& frame);
	void drawInterfaceMeshes(const Frame& frame);
	void drawLandscapeMeshes(const Frame& frame);
}

class StaticMeshComponentManager
{	
private:
	/** Data a single component carries. */
	struct Data
	{
		Data(Entity e, Material* material, Mesh* mesh, std::string material_path, std::string mesh_path)
		: entity(e), material(material), mesh(mesh), material_path(material_path), mesh_path(mesh_path),
			vertex_data(), index_data() {}

		Entity entity;
		Material* material;
		Mesh* mesh;

		GLuint first_index;
		GLuint indices_cnt;
		GLuint base_vertex;

		std::string material_path;
		std::string mesh_path;
		std::vector<uint8_t> vertex_data;
		std::vector<uint32_t> index_data;
	};

	/** Dynamic storage of components data (Since components contains strings, dynamic allocation with a vector is used). */
	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Protection for multi-threaded access */
	mutable std::mutex m_dataAccess_mutex;
	mutable std::mutex m_renderJobAccess_mutex;

	/** StaticMesh RenderJobs */
	RenderJobManager m_renderJobs;

	struct RenderBatch
	{
		ResourceID shader_prgm;
		ResourceID material;
	};
	typedef std::vector<std::vector<Entity>> RenderBatches;
	RenderBatches m_render_batches;

	std::vector<Data>& getData() { return m_data; }

public:

	void addComponent(Entity e, std::string material_path, std::string mesh_path, bool visible = true);

	void addComponent(Entity e, const std::string& material_path, Mesh * mesh, bool visible = true);

	/** Add component using existing GPU resources */
	void addComponent(Entity e, Mesh* mesh, Material* material, bool visible = true);
	
	template<typename VertexContainer, typename IndexContainer>
	void addComponent(Entity e, std::string name, std::string material_path, const VertexContainer& vertices,
						const IndexContainer& indices, const VertexLayout& vertex_description, GLenum mesh_type, bool visible = true)
	{
		std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

		uint idx = static_cast<uint>(m_data.size());
		m_index_map.insert(std::pair<uint,uint>(e.id(), idx));

		m_data.push_back(Data(e,nullptr,nullptr,material_path,name));

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data.back().vertex_data.resize(byte_size);
		std::memcpy(m_data.back().vertex_data.data(),vertices.data(),byte_size);

		m_data.back().index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data.back().index_data.begin());


		// Create GPU resources
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, visible, vertex_description, mesh_type]() {

			std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

			StaticMeshComponentManager::Data& component_data = m_data[idx];

			MaterialInfo mtl_info;
			if (!GEngineCore::resourceManager().checkForMaterial(component_data.material_path))
				mtl_info = ResourceLoading::parseMaterial(component_data.material_path);

			component_data.material = GEngineCore::resourceManager().createMaterial(component_data.material_path, mtl_info.shader_filepaths, mtl_info.texture_filepaths).resource;
			component_data.mesh = GEngineCore::resourceManager().createMesh(component_data.mesh_path,
				component_data.vertex_data,
				component_data.index_data,
				vertex_description,
				mesh_type).resource;
			
			std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
			if(visible)
				m_renderJobs.addRenderJob(RenderJob(m_data[idx].entity, m_data[idx].material, m_data[idx].mesh));

			//TODO clear mesh data from CPU memory?
		});
	}

	template<typename VertexContainer, typename IndexContainer>
	void updateComponent(Entity e, const VertexContainer& vertices, const IndexContainer& indices, const VertexLayout& vertex_description, GLenum mesh_type)
	{
		std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

		auto search = m_index_map.find(e.id());

		assert((search != m_index_map.end()));

		uint idx = search->second;

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data[idx].vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data[idx].vertex_data.data(), vertices.data(), byte_size);

		m_data[idx].index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data[idx].index_data.begin());

		// Update GPU resources
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, vertex_description, mesh_type]() {

			StaticMeshComponentManager::Data& component_data = m_data[idx];

			GEngineCore::resourceManager().updateMesh(component_data.mesh_path,
				component_data.vertex_data,
				component_data.index_data,
				vertex_description,
				mesh_type);

			//TODO clear mesh data from CPU memory?
		});
	}

	void updateComponent(Entity e, Material* material);

	uint getIndex(Entity entity) const;

	RenderJobManager getRenderJobs() const;

	RenderBatches const& getRenderBatches() const { return m_render_batches; }

	void setVisibility(Entity e, bool visible);
};

#endif