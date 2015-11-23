#ifndef LightComponent_h
#define LightComponent_h

#include <unordered_map>

#include "EntityManager.hpp"
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
		Vec3* light_colour;			///< color of the light in rgb values
		float* light_intensity;		///< light intensity...should move towards physically based lighting
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

public:
	LightComponentManager(uint size);
	~LightComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity, Vec3 light_colour, float light_intensity);

	void deleteComonent(Entity entity);

	uint getIndex(Entity entity);

	const Vec3 getColour(uint index);

	const float getIntensity(uint index);

};

#endif