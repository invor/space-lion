#ifndef DebugNameComponent_hpp
#define DebugNameComponent_hpp

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "EntityManager.hpp"

namespace EngineCore
{
	namespace Common
	{
		class NameComponentManager
		{
		private:
			struct Data
			{
				Data(Entity entity, std::string const& debug_name)
					: entity(entity), debug_name(debug_name) {}
				Data(Entity entity, std::string && debug_name)
					: entity(entity), debug_name(std::move(debug_name)) {}

				Entity entity;
				std::string debug_name;
			};

			std::vector<Data> m_data;

			std::unordered_map<uint, uint> m_index_map;

			//std::mutex m_dataAccess_mutex;

		public:
			void addComponent(Entity entity, std::string const& debug_name);
			void addComponent(Entity entity, std::string && debug_name);

			std::pair<bool, uint> getIndex(Entity entity) const;

			std::string getDebugName(Entity entity) const;

			std::string getDebugName(uint index) const;
		};
	}
}

#endif // !DebugNameComponent_hpp
