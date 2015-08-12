#ifndef LightComponent_h
#define LightComponent_h

#include "EntityManager.h"
#include "types.hpp"

class LightComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning that owns the component
		Vec3* lightColour;
	};

public:

	void reallocate(uint size);

	void addComponent(Entity entity);

	void deleteComonent(Entity entity);

	uint getIndex(Entity entity);

};

#endif