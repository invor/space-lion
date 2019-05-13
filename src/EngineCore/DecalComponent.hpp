#ifndef DecalComponent_hpp
#define DecalComponent_hpp

#include <array>
#include <shared_mutex>
#include <unordered_map>

#include "EntityManager.hpp"
#include "ResourceManager.hpp"

class DecalComponentManager
{
private:
	struct Data
	{
		Data(Entity entity, std::array<ResourceID, 4> texture_resources) : m_entity(entity), m_texture_resources(texture_resources) {}

		Entity						m_entity;
		std::string					m_decal_material_name;
		std::array<ResourceID, 4>	m_texture_resources;
		Mat4x4						m_obj_decal_space_transform;
		float						m_decal_width;
		float						m_decal_height;
	};

	struct GPUData
	{
		Mat4x4		decal_space_transform;
		Mat4x4		decal_space_normal_transform;
		uint64_t	decal_texture_handles[4];
	};

	std::vector<Data>			m_data;
	mutable std::shared_mutex	m_data_mutex;
	std::vector<GPUData>		m_gpu_buffer;
	mutable std::shared_mutex	m_gpu_buffer_mutex;
	ResourceID					m_gpu_buffer_resource;

	std::unordered_map<uint, uint>	m_index_map;
	mutable std::shared_mutex		m_index_map_mutex;

	void updateGPUBuffer();

public:
	DecalComponentManager();
	~DecalComponentManager();

	void addComponent(Entity entity, const std::string& material_path, float width = 1.0, float height = 1.0);

	void addComponent(Entity entity, std::array<ResourceID,4> texture_resources, const std::string& material_path = "", float width = 1.0, float height = 1.0);

	void updateComponent(uint eID, const std::string& material_path);

	void updateComponent(uint eID, float width, float height);

	std::pair<bool,uint> getIndex(uint eID);

	std::array<ResourceID, 4> getTextureResources(Entity entity) const;

	std::pair<bool, uint> getIndex(Entity entity) const;

	ResourceID getGPUBufferResource() const;

	std::string getMaterialName(uint eID);

	std::pair<float, float> getDecalSize(uint eID);

	uint getComponentCount();

	std::vector<Entity> getListOfEntities();
};

#endif // !DecalComponent_hpp
