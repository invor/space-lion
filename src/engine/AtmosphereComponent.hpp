#ifndef AtmosphereComponent_hpp
#define AtmosphereComponent_hpp

#include <unordered_map>

#include "EntityManager.hpp"
#include "types.hpp"
#include "MTQueue.hpp"
#include "ResourceManager.h"

/**
 * Manages atmosphere components and functions as a decentralized module of the rendering pipeline
 */
class AtmosphereComponentManager
{
private:
	struct Data
	{
		uint used;						///< number of components currently in use
		uint allocated;					///< number of components that the allocated memery can hold
		void* buffer;					///< raw data pointer

		Entity* entity;					///< entity owning the component

		Vec3* beta_r;					///< extinction coefficient for Rayleigh scattering
		Vec3* beta_m;					///< extinction coefficient for Mie scattering
		float* h_r;						///< ?
		float* h_m;						///< ?
		float* min_altitude;			///< minimum altitude of the atmosphere
		float* max_altitude;			///< maximum altitude of the atmosphere
		AtmosphereMaterial** material;
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	/** Queue to store request for new components until the GPU thread picks them up (see processNewComponents()). */
	MTQueue<uint> m_addedComponents_queue;

	/** Pointer to gfx resource manager */
	ResourceManager* m_resource_mngr;

	/** Add new components, create neceassary resources, compute data. To be called from a thread with OpenGL context! */
	void processNewComponents();

	/** Access raw data. */
	Data const * const getData() const;

	/******************************************************
	 * Compute methods
	 *****************************************************/

	void computeTransmittance(uint index);
	void computeInscatterSingle(uint index);
	void computeIrradianceSingle(uint index);

	/* Grant DeferredRenderingPipeline access to private methods */
	friend class DeferredRenderingPipeline;

public:
	AtmosphereComponentManager(uint size, ResourceManager* resource_mngr);
	~AtmosphereComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity,
						Vec3 beta_r,
						Vec3 beta_m,
						float h_r,
						float h_m,
						float min_altitude,
						float max_altitude);

	const uint getIndex(Entity entity) const;
};

#endif