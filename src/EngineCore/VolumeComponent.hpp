#ifndef VolumeComponent_hpp
#define VolumeComponent_hpp

#include "GlobalEngineCore.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "EntityManager.hpp"
#include "ResourceManager.hpp"

#include "RenderJobs.hpp"
#include "MTQueue.hpp"

#include <unordered_map>

/**
 * Simple component for rendering volume data
 */
class VolumeComponentManager
{
private:
	struct Data
	{
		Data(Entity e, const std::string& volume_path, const std::string& boundingGeometry_path, TextureLayout volume_description, VertexLayout vertex_description, GLenum mesh_type, bool isVisible = true)
			: entity(e), volume_path(volume_path), boundingGeometry_path(boundingGeometry_path),
				volume_description(volume_description), vertex_description(vertex_description), mesh_type(mesh_type), isVisible(isVisible) {}

		Data(Entity e, Texture3D* volume, Mesh* boundingGeometry)
			: entity(e), volume(volume), boundingGeometry(boundingGeometry) {}

		Entity entity;							///< entity

		std::string volume_path;				///< path to raw file or name of volume
		std::string boundingGeometry_path;		///< path to mesh file or name of mesh

		Texture3D* volume;						///< pointer to volume texture
		Mesh* boundingGeometry;					///< pointer to bounding geometry mesh
		GLSLProgram* program;

		std::vector<uint8_t> volume_data;		///< raw volume data
		TextureLayout volume_description;	///< description of volume data

		std::vector<uint8_t> vertex_data;		///< vertex data of bounding geometry
		std::vector<uint32_t> index_data;		///< index data of bounding geometry
		VertexLayout vertex_description;	///< description of bounding geometry vertices
		GLenum mesh_type;
		Vec3 boundingBox_min;
		Vec3 boundingBox_max;

		bool isVisible;
	};

	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Volume RenderJobs */
	RenderJobManager m_renderJobs;

public:
	void registerRenderingPipelineTasks();

	uint getIndex(Entity e);

	/** Add component by loading data from files */
	void addComponent(Entity e, std::string& volume_path, std::string& boundingGeometry_path);

	/** Add component using existing GPU resources */
	void addComponent(Entity e, Texture3D* volume, Mesh* boundingGeometry, const Vec3& boundingBox_min, const Vec3& boundingBox_max);

	/** Add component from raw data */
	template<typename VertexContainer, typename IndexContainer, typename VolumeContainer>
	void addComponent(Entity e, const std::string& volume_name, const VolumeContainer& volume_data, const TextureLayout& volume_description,
						const VertexContainer& vertices, const IndexContainer& indices, const VertexLayout& vertex_description, const GLenum mesh_type,
						const Vec3 boundingBox_min, const Vec3 boundingBox_max, bool isVisible)
	{
		uint idx = static_cast<uint>(m_data.size());

		m_data.push_back(Data(e,volume_name,volume_name+"_bg",volume_description,vertex_description,mesh_type,isVisible));

		// copy volume data
		size_t volume_byte_size = volume_data.size() * sizeof(VolumeContainer::value_type);
		m_data.back().volume_data.resize(volume_byte_size);
		std::memcpy(m_data.back().volume_data.data(),volume_data.data(),volume_byte_size);

		// copy mesh data
		size_t mesh_byte_size = vertices.size() * sizeof(VertexContainer::value_type);
		m_data.back().vertex_data.resize(mesh_byte_size);
		std::memcpy(m_data.back().vertex_data.data(),vertices.data(),mesh_byte_size);
		m_data.back().index_data.resize(indices.size());
		std::copy(indices.begin(), indices.end(), m_data.back().index_data.begin());

		m_data.back().boundingBox_min = boundingBox_min;
		m_data.back().boundingBox_max = boundingBox_max;

		m_index_map.insert(std::pair<uint,uint>(e.id(),idx));


		// Create GPU resources
		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx]() {

			VolumeComponentManager::Data& component_data = m_data[idx];

			GLSLProgram* prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/volRen_v.glsl","../resources/shaders/volRen_f.glsl" }, "generic_volume_rendering").resource;

			component_data.boundingGeometry = GEngineCore::resourceManager().createMesh(component_data.volume_path + "_boundingGeometry", component_data.vertex_data, component_data.index_data, component_data.vertex_description, component_data.mesh_type).resource;
			Texture* volume = GEngineCore::resourceManager().createTexture3D(component_data.volume_path,
																				component_data.volume_description,
																				component_data.volume_data.data()).resource;
			component_data.volume = reinterpret_cast<Texture3D*>( volume );

			Material* mtl = GEngineCore::resourceManager().createMaterial(component_data.volume_path + "_mtl", prgm, { volume }).resource;

			component_data.boundingGeometry = GEngineCore::resourceManager().createMesh(component_data.boundingGeometry_path,
																						component_data.vertex_data,
																						component_data.index_data,
																						component_data.vertex_description,
																						component_data.mesh_type).resource;

			m_renderJobs.addRenderJob(RenderJob(component_data.entity, mtl, component_data.boundingGeometry));
		});
	}

	template<typename VolumeContainer>
	void updateComponent(Entity e, const VolumeContainer& volume_data, const TextureLayout& volume_description)
	{
		uint idx = getIndex(e);

		// copy volume data
		size_t volume_byte_size = volume_data.size() * sizeof(VolumeContainer::value_type);
		m_data[idx].volume_data.resize(volume_byte_size);
		std::memcpy(m_data[idx].volume_data.data(), volume_data.data(), volume_byte_size);

		m_data[idx].volume_description = volume_description;

		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx]() {

			VolumeComponentManager::Data& component_data = m_data[idx];

			component_data.volume->reload(m_data[idx].volume_description, component_data.volume_data.data());

		});
	}

	void setVisibility(Entity e, bool isVisible);

	void draw();
};

#endif