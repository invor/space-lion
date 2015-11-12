#ifndef AtmosphereComponent_hpp
#define AtmosphereComponent_hpp

#include <unordered_map>

#include "EntityManager.hpp"
#include "types.hpp"

class AtmosphereComponentManager
{
private:
	struct Data
	{
		uint used;					///< number of components currently in use
		uint allocated;				///< number of components that the allocated memery can hold
		void* buffer;				///< raw data pointer

		Entity* entity;				///< entity owning the component

		Vec3* beta_r;				///< extinction coefficient for Rayleigh scattering
		Vec3* beta_m;				///< extinction coefficient for Mie scattering
		float* h_r;					///< ?
		float* h_m;					///< ?
		float* min_altitude;		///< minimum altitude of the atmosphere
		float* max_altitude;		///< maximum altitude of the atmosphere
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

public:
	AtmosphereComponentManager(uint size);
	~AtmosphereComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity,
						Vec3 beta_r,
						Vec3 beta_m,
						float h_r,
						float h_m,
						float min_altitude,
						float max_altitude);

	const uint getIndex(Entity entity);
};

#endif