#include "DecalComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"
#include "EditorSelectComponent.hpp"

#include "GlobalRenderingComponents.hpp"
#include "StaticMeshComponent.hpp"
#include "PickingComponent.hpp"

#include "GlobalTools.hpp"
#include "TransformTool.hpp"

#include "GlobalEngineCore.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "ResourceManager.hpp"

DecalComponentManager::DecalComponentManager()
	: m_gpu_buffer_resource(GEngineCore::resourceManager().createSSBOAsync("decal_data_buffer", 0, nullptr))
{
}

DecalComponentManager::~DecalComponentManager()
{
	//TODO
}


void DecalComponentManager::addComponent(Entity entity, const std::string& material_path, float width, float height)
{
	std::array<ResourceID, 4> decal_textures = { GEngineCore::resourceManager().getInvalidResourceID(),
		GEngineCore::resourceManager().getInvalidResourceID(), 
		GEngineCore::resourceManager().getInvalidResourceID(),
		GEngineCore::resourceManager().getInvalidResourceID() };
	std::array<std::string, 4> decal_texture_filepaths = ResourceLoading::parseDecalMaterial(material_path);
	for (int i = 0; i < 4; ++i)
	{
		std::cout << decal_texture_filepaths[i] << std::endl;
		std::vector<unsigned char> image_data;
		TextureLayout image_layout;
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR });
		image_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		ResourceLoading::loadPngImage(decal_texture_filepaths[i], image_data, image_layout);
		decal_textures[i] = GEngineCore::resourceManager().createTexture2D(decal_texture_filepaths[i], image_layout, image_data.data(),true).id;
	}

	addComponent(entity, decal_textures, material_path, width, height);
}

void DecalComponentManager::addComponent(Entity entity, std::array<ResourceID, 4> texture_resources, const std::string& material_path, float width, float height)
{
	uint index = m_data.size();

	std::unique_lock<std::shared_mutex> idx_lock(m_index_map_mutex);
	m_index_map.insert({ entity.id(),index });

	Data new_component(entity, texture_resources);
	new_component.m_decal_material_name = material_path;
	new_component.m_obj_decal_space_transform = glm::scale(new_component.m_obj_decal_space_transform, Vec3(1.0f / width, 1.0f, 1.0f / height));
	new_component.m_decal_width = width;
	new_component.m_decal_height = height;

	std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
	m_data.push_back(new_component);

	std::unique_lock<std::shared_mutex> gpu_data_lock(m_gpu_buffer_mutex);
	GPUData new_gpu_data;
	m_gpu_buffer.push_back(new_gpu_data); // add "empty" dummy data

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, entity](){
		auto query = getIndex(entity);
		if (query.first)
		{
			GPUData new_gpu_data;
			//TODO compute gpu data
			uint transform_idx = GCoreComponents::transformManager().getIndex(entity);
			Mat4x4 world_space_transform = GCoreComponents::transformManager().getWorldTransformation(transform_idx);
			new_gpu_data.decal_space_transform = m_data[query.second].m_obj_decal_space_transform * glm::inverse(world_space_transform);

			int tex_idx = 0;
			for (auto tex_rsrc_entity : m_data[query.second].m_texture_resources)
			{
				auto tex_rsrc = GEngineCore::resourceManager().getTexture(tex_rsrc_entity);

				if (tex_rsrc.state == ResourceState::READY)
				{
					new_gpu_data.decal_texture_handles[tex_idx++] = tex_rsrc.resource->getTextureHandle();
					tex_rsrc.resource->makeResident();

					GLenum err = glGetError();
					if (err != GL_NO_ERROR)
					{
						std::cerr << "GL error during texture handle query for decal: " << err << std::endl;
					}
				}
				else
				{
					std::cerr << "Decal texture resource not ready." << std::endl;
				}
			}

			std::unique_lock<std::shared_mutex> gpu_data_lock(m_gpu_buffer_mutex);
			m_gpu_buffer[query.second] = new_gpu_data;
			auto ssbo_rsrc = GEngineCore::resourceManager().getSSBO(m_gpu_buffer_resource);
			if (ssbo_rsrc.state == ResourceState::READY)
				ssbo_rsrc.resource->reload(m_gpu_buffer);
			else
				std::cerr << "Decal SSBO resource not ready." << std::endl;
		}
	});

#if EDITOR_MODE // preprocessor definition

	VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
		VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3),});

	std::vector<float> vertex_data = { 0.0,0.0,0.0,1.0,0.0,1.0,1.0,
		width,0.0,0.0,1.0,0.0,1.0,1.0,
		0.0,0.0,height,1.0,0.0,1.0,1.0,
		width,0.0,height,1.0,0.0,1.0,1.0 };
	std::vector<uint> index_data = { 0,1, 1,3, 3,2, 2,0 };

	GRenderingComponents::interfaceMeshManager().addComponent<std::vector<float>, std::vector<uint>>(
		entity,
		"decal_" + std::to_string(entity.id()),
		"../resources/materials/editor/interface_cv.slmtl",
		vertex_data,
		index_data,
		vertex_description,
		GL_LINES);

	vertex_data = { 0.0,0.0,0.0,
		width,0.0,0.0,
		0.0,0.0,height,
		width,0.0,height };
	index_data = { 0,1,2,1,3,2 };
	VertexLayout select_proxy_vd(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
	// add selectable component
	GRenderingComponents::pickingManager().addComponent<std::vector<float>, std::vector<uint>>(
		entity,
		"decal_" + std::to_string(entity.id())+"_picking_proxy",
		"../resources/materials/editor/picking.slmtl",
		vertex_data,
		index_data,
		select_proxy_vd,
		GL_TRIANGLES);
	GTools::selectManager().addComponent(entity, [entity]() { GTools::transformTool().activate(); }, [entity]() { GTools::transformTool().deactivate(); }, [this]() {GRenderingComponents::decalManager().updateGPUBuffer(); });
#endif
}

void DecalComponentManager::updateComponent(uint eID, const std::string& material)
{
	std::array<ResourceID, 4> decal_textures = { GEngineCore::resourceManager().getInvalidResourceID(),
		GEngineCore::resourceManager().getInvalidResourceID(),
		GEngineCore::resourceManager().getInvalidResourceID(), 
		GEngineCore::resourceManager().getInvalidResourceID() };
	std::array<std::string, 4> decal_texture_filepaths = ResourceLoading::parseDecalMaterial(material);
	for (int i = 0; i < 4; ++i)
	{
		std::cout << decal_texture_filepaths[i] << std::endl;
		std::vector<unsigned char> image_data;
		TextureLayout image_layout;
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
		image_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR });
		image_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
		ResourceLoading::loadPngImage(decal_texture_filepaths[i], image_data, image_layout);
		decal_textures[i] = GEngineCore::resourceManager().createTexture2D(decal_texture_filepaths[i], image_layout, image_data.data(),true).id;
	}

	uint idx = getIndex(eID).second;
	m_data[idx].m_decal_material_name = material;
	m_data[idx].m_texture_resources = decal_textures;

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, eID, idx]() {
		GPUData new_gpu_data;
		//TODO compute gpu data
		uint transform_idx = GCoreComponents::transformManager().getIndex(eID);
		Mat4x4 world_space_transform = GCoreComponents::transformManager().getWorldTransformation(transform_idx);
		new_gpu_data.decal_space_transform = m_data[idx].m_obj_decal_space_transform * glm::inverse(world_space_transform);
		new_gpu_data.decal_space_normal_transform = Mat4x4(glm::transpose(glm::inverse(glm::mat3(world_space_transform))));

		int tex_idx = 0;
		for (auto tex_rsrc_entity : m_data[idx].m_texture_resources)
		{
			auto tex_rsrc = GEngineCore::resourceManager().getTexture(tex_rsrc_entity);

			if (tex_rsrc.state == ResourceState::READY)
			{
				new_gpu_data.decal_texture_handles[tex_idx++] = tex_rsrc.resource->getTextureHandle();
				tex_rsrc.resource->makeResident();

				GLenum err = glGetError();
				if (err != GL_NO_ERROR)
				{
					std::cerr << "GL error during texture handle query for decal: " << err << std::endl;
				}
			}
			else
			{
				std::cerr << "Decal texture resource not ready." << std::endl;
			}
		}

		std::unique_lock<std::shared_mutex> gpu_data_lock(m_gpu_buffer_mutex);
		m_gpu_buffer[idx] = new_gpu_data;
		auto ssbo_rsrc = GEngineCore::resourceManager().getSSBO(m_gpu_buffer_resource);
		if (ssbo_rsrc.state == ResourceState::READY)
			ssbo_rsrc.resource->reload(m_gpu_buffer);
		else
			std::cerr << "Decal SSBO resource not ready." << std::endl;
		
	});
}

void DecalComponentManager::updateComponent(uint eID, float width, float height)
{
	uint idx = getIndex(eID).second;
	m_data[idx].m_obj_decal_space_transform = glm::scale(Mat4x4(), Vec3(1.0f / width, 1.0f, 1.0f / height));
	m_data[idx].m_decal_width = width;
	m_data[idx].m_decal_height = height;


	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, eID, idx]() {
		GPUData new_gpu_data;
		//TODO compute gpu data
		uint transform_idx = GCoreComponents::transformManager().getIndex(eID);
		Mat4x4 world_space_transform = GCoreComponents::transformManager().getWorldTransformation(transform_idx);
		new_gpu_data.decal_space_transform = m_data[idx].m_obj_decal_space_transform * glm::inverse(world_space_transform);
		new_gpu_data.decal_space_normal_transform = Mat4x4(glm::transpose(glm::inverse(glm::mat3(world_space_transform))));

		int tex_idx = 0;
		for (auto tex_rsrc_entity : m_data[idx].m_texture_resources)
		{
			auto tex_rsrc = GEngineCore::resourceManager().getTexture(tex_rsrc_entity);

			if (tex_rsrc.state == ResourceState::READY)
			{
				new_gpu_data.decal_texture_handles[tex_idx++] = tex_rsrc.resource->getTextureHandle();
				tex_rsrc.resource->makeResident();

				GLenum err = glGetError();
				if (err != GL_NO_ERROR)
				{
					std::cerr << "GL error during texture handle query for decal: " << err << std::endl;
				}
			}
			else
			{
				std::cerr << "Decal texture resource not ready." << std::endl;
			}
		}

		std::unique_lock<std::shared_mutex> gpu_data_lock(m_gpu_buffer_mutex);
		m_gpu_buffer[idx] = new_gpu_data;
		auto ssbo_rsrc = GEngineCore::resourceManager().getSSBO(m_gpu_buffer_resource);
		if (ssbo_rsrc.state == ResourceState::READY)
			ssbo_rsrc.resource->reload(m_gpu_buffer);
		else
			std::cerr << "Decal SSBO resource not ready." << std::endl;

	});


#if EDITOR_MODE // preprocessor definition

	VertexLayout vertex_description(28, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),
		VertexLayout::Attribute(GL_FLOAT,4,GL_FALSE,sizeof(GLfloat) * 3), });

	std::vector<float> vertex_data = { 0.0,0.0,0.0,1.0,0.0,1.0,1.0,
		width,0.0,0.0,1.0,0.0,1.0,1.0,
		0.0,0.0,height,1.0,0.0,1.0,1.0,
		width,0.0,height,1.0,0.0,1.0,1.0 };
	std::vector<uint> index_data = { 0,1, 1,3, 3,2, 2,0 };

	GRenderingComponents::interfaceMeshManager().updateComponent(
		m_data[idx].m_entity,
		vertex_data,
		index_data,
		vertex_description,
		GL_LINES);
	
	vertex_data = { 0.0,0.0,0.0,
		width,0.0,0.0,
		0.0,0.0,height,
		width,0.0,height };
	index_data = { 0,1,2,1,3,2 };
	VertexLayout select_proxy_vd(12, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0) });
	GRenderingComponents::pickingManager().updateComponentProxyGeometry(
		m_data[idx].m_entity,
		vertex_data,
		index_data,
		select_proxy_vd,
		GL_TRIANGLES);
#endif
}

std::pair<bool, uint> DecalComponentManager::getIndex(uint eID)
{
	std::shared_lock<std::shared_mutex> idx_lock(m_index_map_mutex);

	auto search = m_index_map.find(eID);

	if (search != m_index_map.end())
	{
		return std::pair<bool, uint>(true, search->second);
	}
	else
	{
		return std::pair<bool, uint>(false, 0);
	}
}

std::array<ResourceID, 4> DecalComponentManager::getTextureResources(Entity entity) const
{
	auto query = getIndex(entity);

	if (query.first)
	{
		return m_data[query.second].m_texture_resources;
	}
	else
	{
		return std::array<ResourceID, 4>({ GEngineCore::resourceManager().getInvalidResourceID(),
										GEngineCore::resourceManager().getInvalidResourceID(),
										GEngineCore::resourceManager().getInvalidResourceID(),
										GEngineCore::resourceManager().getInvalidResourceID() });
		//TODO find proper solution...
	}
}

std::pair<bool, uint> DecalComponentManager::getIndex(Entity entity) const
{
	std::shared_lock<std::shared_mutex> idx_lock(m_index_map_mutex);
	auto search = m_index_map.find(entity.id());

	std::pair<bool, uint> rtn;

	if (search != m_index_map.end())
	{
		rtn.first = true;
		rtn.second = search->second;
	}
	else
	{
		rtn.first = false;
		rtn.second = 0;
	}

	return rtn;
}

ResourceID DecalComponentManager::getGPUBufferResource() const
{
	return m_gpu_buffer_resource;
}

void DecalComponentManager::updateGPUBuffer()
{
	for (int i=0; i< m_gpu_buffer.size(); ++i)
	{
		uint transform_idx = GCoreComponents::transformManager().getIndex(m_data[i].m_entity);
		Mat4x4 world_space_transform = GCoreComponents::transformManager().getWorldTransformation(transform_idx);
		m_gpu_buffer[i].decal_space_transform = m_data[i].m_obj_decal_space_transform * glm::inverse(world_space_transform);
		m_gpu_buffer[i].decal_space_normal_transform = Mat4x4(glm::transpose(glm::inverse(glm::mat3(world_space_transform))));
	}

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this]() {
		auto ssbo_rsrc = GEngineCore::resourceManager().getSSBO(m_gpu_buffer_resource);
		if (ssbo_rsrc.state == ResourceState::READY)
			ssbo_rsrc.resource->reload(m_gpu_buffer);
		else
			std::cerr << "Decal SSBO resource not ready." << std::endl;
	});
}

std::string DecalComponentManager::getMaterialName(uint eID)
{
	auto query = getIndex(eID);

	if (query.first)
	{
		return m_data[query.second].m_decal_material_name;
	}
	else
	{
		return "";
	}
}

std::pair<float, float> DecalComponentManager::getDecalSize(uint eID)
{
	auto query = getIndex(eID);

	if (query.first)
	{
		return std::pair<float, float>(m_data[query.second].m_decal_width, m_data[query.second].m_decal_height);
	}
	else
	{
		return std::pair<float, float>(0.0,0.0);
	}
}

uint DecalComponentManager::getComponentCount()
{
	return m_data.size();
}

std::vector<Entity> DecalComponentManager::getListOfEntities()
{
	std::vector<Entity> rtn;

	for (auto& decal_data : m_data)
	{
		rtn.push_back(decal_data.m_entity);
	}

	return rtn;
}
