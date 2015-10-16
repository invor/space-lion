#include "RenderJobs.hpp"

RenderJobManager::RenderJobManager(EntityManager* entity_mngr, TransformComponentManager* transform_mngr, CameraComponentManager* camera_mngr, LightComponentManager* light_mngr)
	: m_entities(entity_mngr), m_transform_components(transform_mngr), m_camera_components(camera_mngr), m_light_components(light_mngr)
{
}

RenderJobManager::~RenderJobManager()
{
}

void RenderJobManager::addRenderJob(RenderJob new_job)
{
	for(auto& shader_node : m_root.shaders)
	{
		if(shader_node.shader_prgm == new_job.material->getShaderProgram())
		{

			for(auto& material_node : shader_node.materials)
			{
				if(material_node.material == new_job.material)
				{

					for(auto& mesh_node : material_node.meshes)
					{
						if(mesh_node.mesh == new_job.mesh)
						{
							mesh_node.enities.push_back(new_job.entity);
							mesh_node.instance_count++;

							m_root.num_render_jobs++;

							return;
						}
					}

					/* If we arrived at this point, the mesh isn't in use yet with this specific material. We add it. */
					material_node.meshes.push_back(MeshNode());
					material_node.meshes.back().mesh = new_job.mesh;
					material_node.meshes.back().enities.push_back(new_job.entity);
					material_node.meshes.back().instance_count++;

					m_root.num_render_jobs++;

					return;
				}
			}

			/* If we arrived at this point, the material isn't in use yet. We add it. */
			shader_node.materials.push_back(MaterialNode());
			shader_node.materials.back().material = new_job.material;
			shader_node.materials.back().meshes.push_back(MeshNode());
			shader_node.materials.back().meshes.back().mesh = new_job.mesh;
			shader_node.materials.back().meshes.back().enities.push_back(new_job.entity);
			shader_node.materials.back().meshes.back().instance_count++;

			m_root.num_render_jobs++;

			return;
		}
	}

	/* At this point in the method call, the shader seems not to be in use yet. We add it. */
	m_root.shaders.push_back(ShaderNode());
	auto& new_shader_node = m_root.shaders.back();
	new_shader_node.shader_prgm = new_job.material->getShaderProgram();
	new_shader_node.materials.push_back(MaterialNode());
	auto& new_material_node = new_shader_node.materials.back();
	new_material_node.material = new_job.material;
	new_material_node.meshes.push_back(MeshNode());
	auto& new_mesh_node = new_material_node.meshes.back();
	new_mesh_node.mesh = new_job.mesh;
	new_mesh_node.enities.push_back(new_job.entity);
	new_mesh_node.instance_count++;

	m_root.num_render_jobs++;
}

void RenderJobManager::processRenderJobs(Entity active_camera, std::vector<Entity>& active_lightsources)
{
	/* Get information on active camera */
	Mat4x4 view_matrix = glm::inverse(m_transform_components->getWorldTransformation( m_transform_components->getIndex(active_camera) ));
	Mat4x4 proj_matrix = m_camera_components->getProjectionMatrix( m_camera_components->getIndex(active_camera) );

	for(auto& shader : m_root.shaders)
	{
		/* Bind shader program and set per program uniforms */
		shader.shader_prgm->use();
		shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
		shader.shader_prgm->setUniform("view_matrix", view_matrix);

		int light_counter = 0;

		Vec3 light_position = m_transform_components->getPosition( m_transform_components->getIndex( active_lightsources.front() ) );
		Vec3 light_intensity = m_light_components->getColour( m_light_components->getIndex( active_lightsources.front() ))
								 *m_light_components->getIntensity( m_light_components->getIndex( active_lightsources.front() ));

		shader.shader_prgm->setUniform("lights.position", light_position);
		shader.shader_prgm->setUniform("lights.intensity", light_intensity);

		//shader.shader_prgm->setUniform("lights.position", glm::vec3(2500.0,2500.0,1500.0));
		//shader.shader_prgm->setUniform("lights.intensity", glm::vec3(50000.0));
		shader.shader_prgm->setUniform("num_lights", light_counter);

		for(auto& material : shader.materials)
		{
			material.material->use();

			for(auto& mesh : material.meshes)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for(auto& entity : mesh.enities)
				{
					uint transform_index = m_transform_components->getIndex(entity);
					Mat4x4 model_matrix = m_transform_components->getWorldTransformation(transform_index);

					//	std::cout<<"=========================="<<std::endl;
					//	std::cout<<model_matrix[0][0]<<" "<<model_matrix[1][0]<<" "<<model_matrix[2][0]<<" "<<model_matrix[3][0]<<std::endl;
					//	std::cout<<model_matrix[0][1]<<" "<<model_matrix[1][1]<<" "<<model_matrix[2][1]<<" "<<model_matrix[3][1]<<std::endl;
					//	std::cout<<model_matrix[0][2]<<" "<<model_matrix[1][2]<<" "<<model_matrix[2][2]<<" "<<model_matrix[3][2]<<std::endl;
					//	std::cout<<model_matrix[0][3]<<" "<<model_matrix[1][2]<<" "<<model_matrix[2][3]<<" "<<model_matrix[3][3]<<std::endl;
					//	std::cout<<"=========================="<<std::endl;

					Mat4x4 model_view_matrix = view_matrix * model_matrix;
					//	Mat4x4 model_view_matrix = view_matrix;
					std::string uniform_name("model_view_matrix[" + std::to_string(instance_counter) + "]");
					shader.shader_prgm->setUniform(uniform_name.c_str(), model_view_matrix);

					instance_counter++;

					if(instance_counter == 128)
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

void RenderJobManager::clearRenderJobs()
{
	m_root.shaders.clear();
	m_root.num_render_jobs = 0;
}