#ifndef EditorSelectComponent_hpp
#define EditorSelectComponent_hpp

#include <unordered_map>
#include <shared_mutex>

#include "EntityManager.hpp"

namespace Editor
{
	class SelectComponentManager
	{
	private:
		struct Data
		{
			Data(Entity e, std::function<void()> on_select, std::function<void()> on_deselect, std::function<void()> update)
				: entity(e), selected(false), selectable(true), onSelect(on_select), onDeselect(on_deselect), update(update) {}

			Entity entity;
			bool selected;
			bool selectable;
			std::function<void()> onSelect;
			std::function<void()> onDeselect;
			std::function<void()> update;
		};

		std::vector<Data> m_data;
		mutable std::shared_mutex m_data_mutex;

		std::unordered_map<uint,uint> m_all_selected;
		mutable std::shared_mutex m_all_selected_mutex;

		long m_lastest_selection = -1;

		/** Map for fast entity to component index conversion */
		std::unordered_map<uint, uint> m_index_map;

	public:
		void addComponent(Entity e,
			std::function<void()> on_select = []() {},
			std::function<void()> on_deselect = []() {},
			std::function<void()> update = []() {});

		std::pair<bool, uint> getIndex(uint entity_id) const;

		/**
		 * Exclusive select function. Clears previous selection if new entity is succesfully selected.
		 */
		void select(uint entity_id);

		void addToSelection(uint entity_id);

		void removeFromSelection(uint entity_id);

		void resetSelection();

		void setSelectable(Entity e, bool selectable);

		bool isSelected(Entity e) const;

		bool isSelected(uint entity_id) const;

		std::function<void()> getUpdateFunction(Entity e) const;

		uint getLatestSelectionId() const;

		std::vector<Entity> getAllSelectedEntities() const;

		std::vector<Entity> getAllEntities() const;
	};
}

#endif // !EditorSelectComponent_hpp
