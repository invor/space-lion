#include "EntityManager.hpp"

#include <shared_mutex>

Entity EntityManager::create()
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	Entity new_entity;

	if( (m_is_alive.size()) < MAX_ENTITY_ID)
	{
		m_is_alive.push_back(true);
		new_entity.m_id = static_cast<uint>(m_is_alive.size())-1;
	}
	else
	{
		if(!m_free_indices.empty())
		{
			new_entity.m_id = m_free_indices.front();
			m_free_indices.pop_front();
			m_is_alive[new_entity.m_id] = true;
		}
		else
		{
			//Duh!
		}
	}

	return new_entity;
}

void EntityManager::destroy(Entity entity)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	m_is_alive[entity.m_id] = false;
	m_free_indices.push_back(entity.m_id);

	// TODO //
	// maybe send out a couple of message to component managers etc.
	// that want to immediatly get rid of components (looking at you renderer)
}

uint EntityManager::getEntityCount() const
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	return m_is_alive.size();
}

std::pair<bool, Entity> EntityManager::getEntity(uint index) const
{
	std::pair<bool, Entity> rtn(false,invalidEntity());

	rtn.first = (m_is_alive.size() > index) ? true : false;

	if (rtn.first)
	{
		rtn.second.m_id = index;

		rtn.first = m_is_alive[index];
	}

	return rtn;
}

bool EntityManager::alive(Entity entity) const
{
	return m_is_alive[entity.m_id];
}