#include "VolumeComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"

void VolumeComponentManager::registerRenderingPipelineTasks()
{
	GEngineCore::renderingPipeline().addPerFrameVolumeGpuTask(std::bind(&VolumeComponentManager::draw, this));
}

uint VolumeComponentManager::getIndex(Entity e)
{
	auto search = m_index_map.find(e.id());

	assert( (search != m_index_map.end()) );

	return search->second;
}

void VolumeComponentManager::addComponent(Entity e, std::string& volume_path, std::string& boundingGeometry_path)
{
	//TODO
}

void VolumeComponentManager::addComponent(Entity e, Texture3D* volume, Mesh* boundingGeometry, const Vec3& boundingBox_min, const Vec3& boundingBox_max)
{
	//TODO
	uint idx = static_cast<uint>(m_data.size());
	//m_data.push_back(Data(e, volume->getId(), boundingGeometry->getName(), TextureLayout(), VertexLayout(), GL_TRIANGLES)); //This is potentially dangerous
	m_data.push_back(Data(e, volume->getId(), "", TextureLayout(), VertexLayout(), GL_TRIANGLES)); //This is potentially dangerous

	m_data.back().boundingBox_min = boundingBox_min;
	m_data.back().boundingBox_max = boundingBox_max;

	m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

	Texture* tmp = volume;

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, tmp]() {

		VolumeComponentManager::Data& component_data = m_data[idx];

		GLSLProgram* prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/volRen_v.glsl","../resources/shaders/volRen_f.glsl" }, "generic_volume_rendering").resource;
		
		Material* mtl = GEngineCore::resourceManager().createMaterial(component_data.volume_path + "_mtl", prgm, { tmp }).resource;

		m_renderJobs.addRenderJob(RenderJob(component_data.entity, mtl, component_data.boundingGeometry));
	});
}

void VolumeComponentManager::setVisibility(Entity e, bool isVisible)
{
	uint idx = getIndex(e);

	m_data[idx].isVisible = isVisible;
}

void VolumeComponentManager::draw()
{
	glDisable(GL_CULL_FACE);

	RenderJobManager::RootNode m_root = m_renderJobs.getRoot();

	// Get information on active camera
	Entity active_camera = GCoreComponents::cameraManager().getEntity(GCoreComponents::cameraManager().getActiveCameraIndex());
	Mat4x4 view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(active_camera)));
	Mat4x4 proj_matrix = GCoreComponents::cameraManager().getProjectionMatrix(GCoreComponents::cameraManager().getIndex(active_camera));

	for (auto& shader : m_root.shaders)
	{
		// Bind shader program and set per program uniforms
		shader.shader_prgm->use();

		// TODO upload camera position
		shader.shader_prgm->setUniform("camera_position", GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(active_camera)));

		for (auto& material : shader.materials)
		{
			glActiveTexture(GL_TEXTURE0);
			shader.shader_prgm->setUniform("volume_tx3D", 0);
			material.material->getTextures()[0]->bindTexture();

			for (auto& mesh : material.meshes)
			{
				//	Draw all entities instanced
				int instance_counter = 0;
				std::string uniform_name;

				for (auto& entity : mesh.enities)
				{
					uint idx = getIndex(entity);

					if (!m_data[idx].isVisible)
						continue;

					uint transform_index = GCoreComponents::transformManager().getIndex(entity);
					Mat4x4 model_matrix = GCoreComponents::transformManager().getWorldTransformation(transform_index);

					Vec3 bbox_min = m_data[idx].boundingBox_min;
					Vec3 bbox_max = m_data[idx].boundingBox_max;

					Mat4x4 texture_matrix;// = glm::translate(Mat4x4(), -bbox_min);
					texture_matrix = texture_matrix * glm::scale(Mat4x4(), 1.0f / (bbox_max - bbox_min));
					texture_matrix = texture_matrix * glm::translate(Mat4x4(), -bbox_min);
					texture_matrix = texture_matrix * glm::inverse(model_matrix);

					shader.shader_prgm->setUniform("texture_matrix", texture_matrix);

					Mat4x4 model_view_proj_matrix = proj_matrix * view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_proj_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_proj_matrix);

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

	glEnable(GL_CULL_FACE);
}