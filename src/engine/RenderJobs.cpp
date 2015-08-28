#include "RenderJobs.hpp"

RenderJobManager::RenderJobManager()
{
}

RenderJobManager::~RenderJobManager()
{
}

void RenderJobManager::addRenderJob(Entity entity, std::string material, std::string mesh)
{
	m_job_queue.push(RenderJob(entity,material,mesh));
}

void RenderJobManagerprocessRenderJobs()
{
}