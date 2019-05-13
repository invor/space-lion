#include "EditorSelectComponent.hpp"

#include "GlobalEngineCore.hpp"
#include "TaskSchedueler.hpp"

void Editor::SelectComponentManager::addComponent(Entity e,
	std::function<void()> on_select,
	std::function<void()> on_deselect,
	std::function<void()> update)
{
	std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::unique_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	uint idx = static_cast<uint>(m_data.size());

	m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

	m_data.push_back(Data(e, on_select, on_deselect, update));
}

std::pair<bool, uint> Editor::SelectComponentManager::getIndex(uint entity_id) const
{
	std::shared_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::shared_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	std::pair<bool, uint> rtn;

	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
	{
		rtn.first = false;
		rtn.second = 0;
	}
	else
	{
		rtn.first = true;
		rtn.second = search->second;
	}

	return rtn;
}

void Editor::SelectComponentManager::select(uint entity_id)
{
	if (!getIndex(entity_id).first)
		return;

	resetSelection();

	addToSelection(entity_id);
}

void Editor::SelectComponentManager::addToSelection(uint entity_id)
{
	std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::unique_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
		return;

	if (m_data[search->second].selectable && (!m_data[search->second].selected))
	{
		m_all_selected.insert(std::pair<uint, uint>(entity_id, search->second));
		m_data[search->second].selected = true;
		GEngineCore::taskSchedueler().submitTask([this,search]() {m_data[search->second].onSelect(); });
	}

	m_lastest_selection = entity_id;
}

void Editor::SelectComponentManager::removeFromSelection(uint entity_id)
{
	std::unique_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
		return;

	{
		std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);

		if (m_data[search->second].selected)
		{
			m_data[search->second].selected = false;
			m_all_selected.erase(entity_id);
			GEngineCore::taskSchedueler().submitTask([this, search]() {m_data[search->second].onDeselect(); });
		}

		if (entity_id == m_lastest_selection)
			m_lastest_selection = -1;
	}
}

void Editor::SelectComponentManager::resetSelection()
{
	{
		std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
		std::unique_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);
	
		for (auto& entry : m_all_selected)
		{
			m_data[entry.second].selected = false;
			GEngineCore::taskSchedueler().submitTask([this, entry]() {m_data[entry.second].onDeselect(); });
		}
	
		m_all_selected.clear();
	
		m_lastest_selection = -1;
	}
}

void Editor::SelectComponentManager::setSelectable(Entity e, bool selectable)
{
	std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::shared_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(e.id());

	if (search == m_index_map.end())
		return;

	m_data[search->second].selectable = selectable;
}

bool Editor::SelectComponentManager::isSelected(Entity e) const
{
	std::unique_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::shared_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(e.id());

	if (search == m_index_map.end())
		return false;

	return m_data[search->second].selected;
}

bool Editor::SelectComponentManager::isSelected(uint entity_id) const
{
	std::shared_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::shared_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(entity_id);

	if (search == m_index_map.end())
		return false;

	return m_data[search->second].selected;
}

std::function<void()> Editor::SelectComponentManager::getUpdateFunction(Entity e) const
{
	std::shared_lock<std::shared_mutex> data_lock(m_data_mutex);
	std::shared_lock<std::shared_mutex> all_selected_lock(m_all_selected_mutex);

	auto search = m_index_map.find(e.id());

	if (search == m_index_map.end())
		return []() {};

	return m_data[search->second].update;
}

uint Editor::SelectComponentManager::getLatestSelectionId() const
{
	return m_lastest_selection;
}

std::vector<Entity> Editor::SelectComponentManager::getAllSelectedEntities() const
{
	std::vector<Entity> rtn;

	rtn.reserve(m_data.size());

	std::shared_lock<std::shared_mutex> data_lock(m_data_mutex);

	for (auto& data : m_data)
	{
		if(data.selected)
			rtn.push_back(data.entity);
	}

	return rtn;
}

std::vector<Entity> Editor::SelectComponentManager::getAllEntities() const
{
	std::vector<Entity> rtn;

	rtn.reserve(m_data.size());

	std::shared_lock<std::shared_mutex> data_lock(m_data_mutex);

	for (auto& data : m_data)
			rtn.push_back(data.entity);

	return rtn;
}