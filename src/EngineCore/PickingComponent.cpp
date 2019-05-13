#include "PickingComponent.hpp"

#include "GlobalEngineCore.hpp"

#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "RenderJobs.hpp"
#include "ResourceLoading.hpp"

void PickingComponentManager::registerRenderingPipelineTasks()
{
	GEngineCore::renderingPipeline().addPerFramePickingGpuTask(std::bind(&PickingComponentManager::draw, this));
}

void PickingComponentManager::addComponent(Entity e, std::string material_path, std::string mesh_path,
											std::function<void()> on_pick)
{
	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

	uint idx = static_cast<uint>(m_data.size());

	m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

	m_data.push_back(Data(e, material_path, mesh_path, VertexLayout(0, {}), GL_TRIANGLES, on_pick));

	ResourceLoading::loadFbxGeometry(mesh_path, m_data.back().vertex_data, m_data.back().index_data, m_data.back().vertex_description);

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx] { this->bufferComponentData(idx); });
}

void PickingComponentManager::setPickable(Entity e, bool pickable)
{
	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

	auto search = m_index_map.find(e.id());

	if (search == m_index_map.end())
		return;

	m_data[search->second].pickable = pickable;
}

std::function<void()> PickingComponentManager::getOnPickFunction(uint entity_id)
{
	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
		return []() {};

	return m_data[search->second].onPick;
}

void PickingComponentManager::bufferComponentData(uint idx)
{
	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

	MaterialInfo mtl_info = ResourceLoading::parseMaterial(m_data[idx].material_path);
	Material* material = GEngineCore::resourceManager().createMaterial(m_data[idx].material_path, mtl_info.shader_filepaths, mtl_info.texture_filepaths).resource;
	//std::shared_ptr<Material> material = GEngineCore::resourceManager().createMaterial(m_data[idx].material_path);
	//std::shared_ptr<Mesh> mesh = GEngineCore::resourceManager().createMesh(component_data->mesh_path);
	Mesh* mesh = GEngineCore::resourceManager().createMesh(m_data[idx].mesh_path,
																				m_data[idx].vertex_data,
																				m_data[idx].index_data,
																				m_data[idx].vertex_description,
																				m_data[idx].mesh_type).resource;

	std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
	m_picking_pass.addRenderJob(RenderJob(m_data[idx].entity, material, mesh));
}

void PickingComponentManager::updateComponentData(uint idx)
{
	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);

	GEngineCore::resourceManager().updateMesh(m_data[idx].mesh_path,
		m_data[idx].vertex_data,
		m_data[idx].index_data,
		m_data[idx].vertex_description,
		m_data[idx].mesh_type);
}

void PickingComponentManager::draw()
{
	Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());

	std::unique_lock<std::mutex> data_lock(m_dataAccess_mutex);
	std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
	RenderJobManager::RootNode m_root = m_picking_pass.getRoot();

	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_camera)));
	Mat4x4 proj_matrix = GCoreComponents::cameraManager().getProjectionMatrix(GCoreComponents::cameraManager().getIndex(active_camera));

	for (auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);

		for (auto& material : shader.materials)
		{
			for (auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for (auto& entity : mesh.enities)
				{
					if (!m_data[m_index_map.find(entity.id())->second].pickable)
						continue;

					uint transform_index = GCoreComponents::transformManager().getIndex(entity);
					Mat4x4 model_matrix = GCoreComponents::transformManager().getWorldTransformation(transform_index);
					Mat4x4 model_view_matrix = view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_matrix);

					std::string id_uniform_name("entity[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(id_uniform_name.c_str(), (int)entity.id());

					instance_counter++;

					if (instance_counter == 128)
					{
						mesh.mesh->draw(instance_counter);
						instance_counter = 0;
					}
				}
				mesh.mesh->draw(instance_counter);
				instance_counter = 0;
			}
		}
	}
}