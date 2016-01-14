#ifndef SelectComponent_hpp
#define SelectComponent_hpp

#include "EntityManager.hpp"

class SelectComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;
		bool* selected;
	};

	Data m_data;
public:
	SelectComponentManager(uint size);
	~SelectComponentManager();

	void reallocate(uint size);

};

#endif