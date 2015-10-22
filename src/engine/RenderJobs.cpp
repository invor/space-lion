#include "RenderJobs.hpp"

RenderJobManager::RenderJobManager()
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

RenderJobManager::RootNode RenderJobManager::getRoot()
{
	return m_root;
}

void RenderJobManager::clearRenderJobs()
{
	m_root.shaders.clear();
	m_root.num_render_jobs = 0;
}