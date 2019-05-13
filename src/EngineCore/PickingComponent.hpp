#include "GlobalCoreComponents.hpp"

#ifndef PickingComponent_hpp
#define PickingComponent_hpp

#include <list>

#include "EntityManager.hpp"
#include "ResourceManager.hpp"
#include "RenderJobs.hpp"

class PickingComponentManager
{
private:
	struct Data
	{
		Data(Entity e, std::string material_path, std::string mesh_path, VertexLayout vertex_description, GLenum mesh_type,
			std::function<void()> on_pick)
			: entity(e), pickable(true), material_path(material_path), mesh_path(mesh_path),
			vertex_data(), index_data(), vertex_description(vertex_description), mesh_type(mesh_type),
			onPick(on_pick) {}

		Entity entity;
		bool pickable;
		std::function<void()> onPick;

		std::string material_path;
		std::string mesh_path;
		std::vector<uint8_t> vertex_data;
		std::vector<uint32_t> index_data;
		VertexLayout vertex_description;
		GLenum mesh_type;
	};

	std::vector<Data> m_data;
	mutable std::mutex m_dataAccess_mutex;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Render jobs for a picking pass */
	RenderJobManager m_picking_pass;
	mutable std::mutex m_renderJobAccess_mutex;

	/*****************************************************************
	* Buffer and draw methods (only call from GPU thread!)
	****************************************************************/
	void bufferComponentData(uint idx);
	void updateComponentData(uint idx);
	void draw();

public:
	void registerRenderingPipelineTasks();

	void addComponent(Entity e, std::string material_path, std::string mesh_path,
						std::function<void()> on_pick = [](){});

	template<typename VertexContainer, typename IndexContainer>
	void addComponent(Entity e, std::string name, std::string material_path, const VertexContainer& vertices,
		const IndexContainer& indices, const VertexLayout& vertex_description, GLenum mesh_type,
		std::function<void()> on_pick = []() {})
	{
		std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

		uint idx = static_cast<uint>(m_data.size());

		m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

		m_data.push_back(Data(e, material_path, name, vertex_description, mesh_type,on_pick));

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data.back().vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data.back().vertex_data.data(), vertices.data(), byte_size);

		m_data.back().index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data.back().index_data.begin());

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx] { this->bufferComponentData(idx); });
	}

	template<typename VertexContainer, typename IndexContainer>
	void updateComponentProxyGeometry(Entity e, const VertexContainer& vertices,
		const IndexContainer& indices, const VertexLayout& vertex_description, GLenum mesh_type)
	{
		auto search = m_index_map.find(e.id());

		if (search == m_index_map.end())
			return;

		uint idx = search->second;

		size_t byte_size = vertices.size() * sizeof(VertexContainer::value_type);

		m_data[idx].vertex_data.resize(vertices.size() * sizeof(VertexContainer::value_type));
		std::memcpy(m_data[idx].vertex_data.data(), vertices.data(), byte_size);

		m_data[idx].index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data[idx].index_data.begin());

		m_data[idx].vertex_description = vertex_description;

		m_data[idx].mesh_type = mesh_type;

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx] { this->updateComponentData(idx); });
	}

	void setPickable(Entity e, bool pickable);

	std::function<void()> getOnPickFunction(uint entity_id);
};

#endif