#ifndef SunlightComponent_h
#define SunlightComponent_h

#include <unordered_map>

#include "EntityManager.hpp"
#include "types.hpp"
#include "MTQueue.hpp"

class SunlightComponentManager
{
private:
	struct Data
	{
		uint used;					///< number of components currently in use
		uint allocated;				///< number of components that the allocated memery can hold
		void* buffer;				///< raw data pointer

		Entity* entity;				///< entity owning that owns the component
		Vec3* light_colour;			///< color of the light in rgb values
		float* lumen;				///< Luminous power of the light source given in Lumen (lm)
		float* star_radius;			///< Radius of the star (used to compute solid angle)
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	MTQueue<Entity> m_added_components_queue;

	/** Access raw data. */
	Data const * const getData() const { return &m_data; }
	MTQueue<Entity>& getComponentsQueue()  { return m_added_components_queue; }

	friend class DeferredRenderingPipeline;

public:
	SunlightComponentManager(uint size);
	~SunlightComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

	void deleteComonent(Entity entity);

	uint getComponentCount() { return m_data.used; }

	uint getIndex(Entity entity);

	const Vec3 getColour(uint index);

	const float getLumen(uint index);

	const float getStarRadius(uint index);

};

#endif