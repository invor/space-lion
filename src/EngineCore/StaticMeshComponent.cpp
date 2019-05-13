#include "StaticMeshComponent.hpp"

#include "Frame.hpp"

#include "CameraComponent.hpp"
#include "ResourceLoading.hpp"

// TODO Move actual draw functions elsewhere?
namespace MeshComponent
{
	void drawStaticMeshes(const Frame& frame)
	{
		auto& root = frame.m_staticMesh_renderJobs.getRoot();

		// Get information on active camera
		Mat4x4 view_matrix = frame.m_view_matrix;
		//Mat4x4 proj_matrix = frame.m_projection_matrix;
		Mat4x4 proj_matrix = glm::perspective(frame.m_fovy, frame.m_aspect_ratio, 0.01f, 10000.0f);

		for (auto& shader : root.shaders)
		{
			// Bind shader program and set per program uniforms
			shader.shader_prgm->use();
			shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
			shader.shader_prgm->setUniform("view_matrix", view_matrix);

			for (auto& material : shader.materials)
			{
				int num_tex = static_cast<int>(material.material->getTextures().size());

				for (int i = 0; i < num_tex; i++)
				{
					glActiveTexture(GL_TEXTURE0 + i);
					std::string uniform = "tx2D_" + std::to_string(i);
					shader.shader_prgm->setUniform(uniform.c_str(), i);
					material.material->getTextures()[i]->bindTexture();
				}

				for (auto& mesh : material.meshes)
				{
					// Draw all entities instanced
					int instance_counter = 0;
					std::string uniform_name;

					for (auto& transfrom_idx : mesh.transform_indices)
					{
						Mat4x4 model_matrix = frame.m_transforms[transfrom_idx];

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

						if (instance_counter == 128)
						{
							mesh.mesh->draw(instance_counter);
							instance_counter = 0;
						}
					}

					if(mesh.mesh->getPrimitiveType() == GL_PATCHES)
						glPatchParameteri(GL_PATCH_VERTICES, 4);

					mesh.mesh->draw(instance_counter);
					instance_counter = 0;
				}
			}
		}
	}

	void drawInterfaceMeshes(const Frame& frame)
	{
		auto& m_root = frame.m_interfaceMesh_renderJobs.getRoot();

		// Get information on active camera
		Mat4x4 view_matrix = frame.m_view_matrix;
		//Mat4x4 proj_matrix = frame.m_projection_matrix;
		Mat4x4 proj_matrix = glm::perspective(frame.m_fovy, frame.m_aspect_ratio, 0.01f, 10000.0f);

		for (auto& shader : m_root.shaders)
		{
			// Bind shader program and set per program uniforms
			shader.shader_prgm->use();
			shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
			shader.shader_prgm->setUniform("view_matrix", view_matrix);

			for (auto& material : shader.materials)
			{
				int num_tex = static_cast<int>(material.material->getTextures().size());

				for (int i = 0; i < num_tex; i++)
				{
					glActiveTexture(GL_TEXTURE0 + i);
					std::string uniform = "tx2D_" + std::to_string(i);
					shader.shader_prgm->setUniform(uniform.c_str(), i);
					material.material->getTextures()[i]->bindTexture();
				}

				for (auto& mesh : material.meshes)
				{
					// Draw all entities instanced
					int instance_counter = 0;
					std::string uniform_name;

					for (auto& transfrom_idx : mesh.transform_indices)
					{
						Mat4x4 model_matrix = frame.m_transforms[transfrom_idx];

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

						if (instance_counter == 128)
						{
							mesh.mesh->draw(instance_counter);
							instance_counter = 0;
						}
					}

					glPatchParameteri(GL_PATCH_VERTICES, 3);

					mesh.mesh->draw(instance_counter);
					instance_counter = 0;
				}
			}
		}
	}

	void drawLandscapeMeshes(const Frame& frame)
	{
		auto& root = frame.m_landscapeMesh_renderJobs.getRoot();

		// Get information on active camera
		Mat4x4 view_matrix = frame.m_view_matrix;
		//Mat4x4 proj_matrix = frame.m_projection_matrix;
		Mat4x4 proj_matrix = glm::perspective(frame.m_fovy, frame.m_aspect_ratio, 0.01f, 10000.0f);

		for (auto& shader : root.shaders) // TODO redundant for landscapes - there will only be a single shader
		{
			// Bind shader program and set per program uniforms
			shader.shader_prgm->use();
			shader.shader_prgm->setUniform("projection_matrix", proj_matrix);
			shader.shader_prgm->setUniform("view_matrix", view_matrix);

			for (auto& material : shader.materials)
			{
				// TODO bindless textures or bind as arrays
				//material.material->

				for (auto& mesh : material.meshes)
				{
					for (auto& transfrom_idx : mesh.transform_indices) // TODO redundant for landscapes - each mesh is unique
					{
						Mat4x4 model_matrix = frame.m_transforms[transfrom_idx];
						Mat4x4 model_view_matrix = view_matrix * model_matrix;

						shader.shader_prgm->setUniform("model_view_matrix", model_view_matrix);

						if (mesh.mesh->getPrimitiveType() == GL_PATCHES)
							glPatchParameteri(GL_PATCH_VERTICES, 4);

						mesh.mesh->draw();
					}
				}
			}
		}
	}
}

void StaticMeshComponentManager::addComponent(Entity e, std::string material_path, std::string mesh_path, bool visible)
{
	std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

	uint idx = static_cast<uint>(m_data.size());

	m_index_map.insert(std::pair<uint,uint>(e.id(),idx));

	m_data.push_back(Data(e,nullptr,nullptr,material_path,mesh_path));

	VertexLayout vertex_desc;

	if( ! GEngineCore::resourceManager().checkForMesh(mesh_path))
		ResourceLoading::loadFbxGeometry(mesh_path,m_data.back().vertex_data,m_data.back().index_data, vertex_desc);

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this,e,visible, vertex_desc]() {

		std::unique_lock<std::mutex> lock(m_dataAccess_mutex);
		
		uint idx = getIndex(e); // get index at time of execution (data might have been reallocated in the mean time)

		StaticMeshComponentManager::Data& component_data = m_data[idx];

		MaterialInfo mtl_info;
		if(! GEngineCore::resourceManager().checkForMaterial(component_data.material_path))
			mtl_info = ResourceLoading::parseMaterial(component_data.material_path);

		component_data.material = GEngineCore::resourceManager().createMaterial(component_data.material_path, mtl_info.shader_filepaths, mtl_info.texture_filepaths).resource;
		//component_data.material = GEngineCore::resourceManager().createMaterial(component_data.material_path).get();
		component_data.mesh = GEngineCore::resourceManager().createMesh(component_data.mesh_path,
			component_data.vertex_data,
			component_data.index_data,
			vertex_desc,
			GL_TRIANGLES).resource;

		std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
		if(visible)
			m_renderJobs.addRenderJob(RenderJob(m_data[idx].entity, m_data[idx].material, m_data[idx].mesh));

		//TODO clear mesh data from CPU memory?
	});
}

void StaticMeshComponentManager::addComponent(Entity e, const std::string& material_path, Mesh * mesh, bool visible)
{
	std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

	uint idx = static_cast<uint>(m_data.size());

	m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

	m_data.push_back(Data(e, nullptr, mesh, material_path, ""));
	//m_data.push_back(Data(e, nullptr, mesh, material_path, mesh->getName(), VertexLayout(0, {}), GL_TRIANGLES));


	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, e, visible]() {

		std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

		uint idx = getIndex(e); // get index at time of execution (data might have been reallocated in the mean time)

		StaticMeshComponentManager::Data& component_data = m_data[idx];

		MaterialInfo mtl_info;
		if (!GEngineCore::resourceManager().checkForMaterial(component_data.material_path))
			mtl_info = ResourceLoading::parseMaterial(component_data.material_path);

		component_data.material = GEngineCore::resourceManager().createMaterial(component_data.material_path, mtl_info.shader_filepaths, mtl_info.texture_filepaths).resource;
		//component_data.material = GEngineCore::resourceManager().createMaterial(component_data.material_path).get();


		std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
		if(visible)
			m_renderJobs.addRenderJob(RenderJob(m_data[idx].entity, m_data[idx].material, m_data[idx].mesh));

		//TODO clear mesh data from CPU memory?
	});
}

void StaticMeshComponentManager::addComponent(Entity e, Mesh* mesh, Material* material, bool visible)
{
	std::unique_lock<std::mutex> lock(m_dataAccess_mutex);

	uint idx = static_cast<uint>(m_data.size());

	m_index_map.insert(std::pair<uint,uint>(e.id(),idx));

	//TODO not that it matters but GL_TRIANGLES could be wrong in some cases...so better query the type from mesh
	//m_data.push_back(Data(e,material,mesh,material->getId(),mesh->getName(),VertexLayout(0,{}),GL_TRIANGLES));
	m_data.push_back(Data(e, material, mesh, material->getId(),""));

	StaticMeshComponentManager::Data& component_data = m_data.back();


	std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);
	if(visible)
		m_renderJobs.addRenderJob(RenderJob(component_data.entity, component_data.material, component_data.mesh));
}

void StaticMeshComponentManager::updateComponent(Entity e, Material* material)
{
	std::unique_lock<std::mutex> renderJob_lock(m_renderJobAccess_mutex);

	uint idx = getIndex(e);

	m_renderJobs.removeRenderJob(RenderJob(e, m_data[idx].material, m_data[idx].mesh));

	m_data[idx].material = material;

	m_renderJobs.addRenderJob(RenderJob(e, m_data[idx].material, m_data[idx].mesh));
}

uint StaticMeshComponentManager::getIndex(Entity entity) const
{
	auto search = m_index_map.find(entity.id());

	assert((search != m_index_map.end()));

	return search->second;
}

RenderJobManager StaticMeshComponentManager::getRenderJobs() const
{
	std::unique_lock<std::mutex> lock(m_renderJobAccess_mutex);

	return m_renderJobs;
}

void StaticMeshComponentManager::setVisibility(Entity e, bool visible)
{
	// TODO more efficient implementation
	std::unique_lock<std::mutex> lock(m_renderJobAccess_mutex);

	uint idx = getIndex(e);

	if (visible)
	{
		m_renderJobs.addRenderJob(RenderJob(m_data[idx].entity, m_data[idx].material, m_data[idx].mesh));
	}
	else
	{
		m_renderJobs.removeRenderJob(RenderJob(m_data[idx].entity, m_data[idx].material, m_data[idx].mesh));
	}
}