#ifndef VolumeComponent_hpp
#define VolumeComponent_hpp

#include <unordered_map>

#include "EntityManager.hpp"
#include "ResourceManager.h"
#include "MTQueue.hpp"

/**
 * Simple component for rendering volume data
 */
class VolumeComponentManager
{
private:
	struct Data
	{
		Data(Entity e, const std::string& volume_path, const std::string& boundingGeometry_path, TextureDescriptor volume_description, VertexDescriptor vertex_description, GLenum mesh_type)
			: entity(e), volume_path(volume_path), boundingGeometry_path(boundingGeometry_path),
				volume_description(volume_description), vertex_description(vertex_description), mesh_type(mesh_type) {}

		Data(Entity e, std::shared_ptr<Texture3D> volume, std::shared_ptr<Mesh> boundingGeometry)
			: entity(e), volume(volume), boundingGeometry(boundingGeometry) {}

		Entity entity;							///< entity

		std::string volume_path;				///< path to raw file or name of volume
		std::string boundingGeometry_path;		///< path to mesh file or name of mesh

		std::shared_ptr<Texture3D> volume;		///< pointer to volume texture
		std::shared_ptr<Mesh> boundingGeometry;	///< pointer to bounding geometry mesh

		std::vector<uint8_t> volume_data;		///< raw volume data
		TextureDescriptor volume_description;	///< description of volume data

		std::vector<uint8_t> vertex_data;		///< vertex data of bounding geometry
		std::vector<uint32_t> index_data;		///< index data of bounding geometry
		VertexDescriptor vertex_description;	///< description of bounding geometry vertices
		GLenum mesh_type;
		Vec3 boundingBox_min;
		Vec3 boundingBox_max;
	};

	std::vector<Data> m_data;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint,uint> m_index_map;

	/** Thread safe queue containing indices of added components that haven't been registerd by the Rendering Pipeline yet. */
	MTQueue<uint> m_update_queue;

	/** Pointer to active ResourceManager */
	ResourceManager* m_resource_mngr;

	/* Grant Rendering Pipeline access to private members. */
	friend class DeferredRenderingPipeline;

public:
	VolumeComponentManager();
	~VolumeComponentManager();

	uint getIndex(Entity e);

	/** Add component by loading data from files */
	void addComponent(Entity e, std::string& volume_path, std::string& boundingGeometry_path);

	/** Add component using existing GPU resources */
	void addComponent(Entity e, std::shared_ptr<Texture3D> volume, std::shared_ptr<Mesh> boundingGeometry);

	/** Add component from raw data */
	template<typename VertexContainer, typename IndexContainer, typename VolumeContainer>
	void addComponent(Entity e, const std::string& volume_name, const VolumeContainer& volume_data, const TextureDescriptor& volume_description,
						const VertexContainer& vertices, const IndexContainer& indices, const VertexDescriptor& vertex_description, const GLenum mesh_type,
						const Vec3 boundingBox_min, const Vec3 boundingBox_max)
	{
		uint idx = m_data.size();

		m_data.push_back(Data(e,volume_name,volume_name+"_bg",volume_description,vertex_description,mesh_type));

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
		m_update_queue.push(idx);
	}
};

#endif