#include "PtexMeshComponent.hpp"

#include "Frame.hpp"
#include "GLSLProgram.h"
#include "Mesh.hpp"
#include "shaderStorageBufferObject.h"

namespace PtexMeshComponent
{
	void submitPtexMeshRenderJobs(const Frame& frame)
	{
		const PtexMeshRenderJobs& render_jobs = frame.m_landscapePtex_renderJobs;

		// Get information on active camera
		Mat4x4 view_matrix = frame.m_view_matrix;
		//Mat4x4 proj_matrix = frame.m_projection_matrix;
		Mat4x4 proj_matrix = glm::perspective(frame.m_fovy, frame.m_aspect_ratio, 0.01f, 10000.0f);

		for (auto batch : render_jobs)
		{
			GLSLProgram* prgm = batch.first;

			// TODO use program
			prgm->use();

			//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			// TODO upload per program data
			prgm->setUniform("projection_matrix", proj_matrix);

			for (auto& batch_data : batch.second)
			{
				// TODO bind per object ptex resources
				batch_data.ptex_resources.ptex_bindless_texture_handles->bind(2);
				batch_data.ptex_resources.ptex_params->bind(3);
				// TODO upload index of transform matrix?
				//prgm->setUniform("transform_idx", batch_data.transform_idx);
				Mat4x4 model_matrix = frame.m_transforms[batch_data.transform_idx];
				model_matrix = Mat4x4(1.0f);
				Mat4x4 model_view_matrix = view_matrix * model_matrix;
				prgm->setUniform("model_view_matrix", model_view_matrix);

				prgm->setUniform("textures_cnt", (batch_data.ptex_resources.ptex_bindless_texture_handles->getSize() / 8u));
				prgm->setUniform("patch_cnt", (batch_data.mesh->getIndicesCount()/4u));

				if (batch_data.mesh->getPrimitiveType() == GL_PATCHES)
					glPatchParameteri(GL_PATCH_VERTICES, 4);
				// TODO submit draw call
				batch_data.mesh->draw();
			}
		}
		
	}
}


void PtexMeshComponentManager::addComponent(
	Entity e,
	ResourceID mesh,
	ResourceID material,
	ResourceID ptex_parameters,
	ResourceID ptex_bindless_texture_handles,
	bool visible)
{
	uint idx = static_cast<uint>(m_data.size());
	m_index_map.insert({ e.id(),idx });
	m_data.push_back(Data(e, mesh, material, ptex_parameters, ptex_bindless_texture_handles, visible));
}

std::pair<bool, uint> PtexMeshComponentManager::getIndex(Entity e) const
{
	auto search = m_index_map.find(e.id());

	std::pair<bool, uint> rtn;

	rtn.first = (search != m_index_map.end()) ? true : false;
	rtn.second = search->second;

	return rtn;
}

void PtexMeshComponentManager::setVisibility(Entity e, bool visible)
{
	auto idx_search = getIndex(e);

	if (idx_search.first)
	{
		m_data[idx_search.second].visible = visible;
	}
}

void PtexMeshComponentManager::setPtexParameters(Entity e, ResourceID ptex_params)
{
	auto idx_search = getIndex(e);

	if (idx_search.first)
	{
		m_data[idx_search.second].ptex_parameters = ptex_params;
	}
}

ResourceID PtexMeshComponentManager::getMesh(uint idx)
{
	return m_data[idx].mesh;
}

ResourceID PtexMeshComponentManager::getMaterial(uint idx)
{
	return m_data[idx].material;
}

ResourceID PtexMeshComponentManager::getPtexParameters(uint idx)
{
	return m_data[idx].ptex_parameters;
}

ResourceID PtexMeshComponentManager::getPtexBindTextureHandles(uint idx)
{
	return m_data[idx].ptex_bindless_texture_handles;
}

std::vector<Entity> PtexMeshComponentManager::getListOfEntities() const
{
	std::vector<Entity> rtn;

	for (auto& data : m_data)
		rtn.push_back(data.entity);

	return rtn;
}