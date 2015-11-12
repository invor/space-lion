#include "ComputeJobs.hpp"

ComputeJobManager::ComputeJobManager()
{
}

ComputeJobManager::~ComputeJobManager()
{
}

void ComputeJobManager::addOneshotComputeJob(ComputeJob new_job)
{
	m_oneshot_jobs.push(new_job);
}

void ComputeJobManager::addRepeatingComputeJob(ComputeJob new_job)
{
	m_repeating_jobs.push_back(new_job);
}

void ComputeJobManager::executeOneshotComputerJobs()
{
}

void ComputeJobManager::executeRepeatingComputeJobs()
{
}