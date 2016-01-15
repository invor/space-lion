#ifndef PointlightComponent_h
#define PointlightComponent_h

#include <unordered_map>

#include "EntityManager.hpp"
#include "types.hpp"
#include "MTQueue.hpp"

class PointlightComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning that owns the component
		Vec3* light_colour;			///< color of the light in rgb values
		float* lumen;				///< Luminous power of the light source given in Lumen (lm)
		float* radius;				///< Maximum radius
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	MTQueue<Entity> m_added_components_queue;

	/** Access raw data. */
	Data const * const getData() const;
	MTQueue<Entity>& getComponentsQueue()  { return m_added_components_queue; }

	friend class DeferredRenderingPipeline;

public:
	PointlightComponentManager(uint size);
	~PointlightComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity, Vec3 light_colour, float lumen, float radius);

	void deleteComonent(Entity entity);



	uint getIndex(Entity entity);

	const Vec3 getColour(uint index);

	const float getLumen(uint index);

	const float getRadius(uint index);

};

#endif