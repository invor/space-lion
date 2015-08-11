#include "EntityManager.h"


Entity EntityManager::create()
{
	Entity new_entity;

	if( (m_is_alive.size()-1) < MAX_UINT)
	{
		m_is_alive.push_back(true);
		new_entity.m_id = m_is_alive.size()-1;
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
	m_is_alive[entity.m_id] = false;
	m_free_indices.push_back(entity.m_id);

	// TODO //
	// maybe send out a couple of message to component managers etc.
	// that want to immediatly get rid of components (looking at you renderer)
}

bool EntityManager::alive(Entity entity) const
{
	return m_is_alive[entity.m_id];
}