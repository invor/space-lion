#ifndef AirplanePhysicsComponent_hpp
#define AirplanePhysicsComponent_hpp

#include <unordered_map>

#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "types.hpp"

class AirplanePhysicsComponentManager
{	
private:
	struct Data
	{
		uint used;					///< number of components currently in use
		uint allocated;				///< number of components that the allocated memery can hold
		void* buffer;				///< raw data pointer

		Entity* entity;				///< entity owning the component

		Vec3* velocity;				///< velocity of the airplane (direction and magnitude)
		Vec3* acceleration;			///< acceleration of the airplance

		float* engine_thrust;		///< engine thrust, force acting in the direction of airplane orientation
		float* aerodynamic_drag;	///< aerodynamic drag, force acting in opposite direction of the velocity
		float* aerodynamic_lift;	///< aerodynamic lift, force acting upward from airplane orientation

		float* wing_surface;		///< airplane wing surface (necessary for computing lift)
		float* mass;				///< airplane mass (necessary for computing lift)

	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	TransformComponentManager* m_transform_mngr;

public:
	AirplanePhysicsComponentManager(uint size, TransformComponentManager* transform_mngr);
	~AirplanePhysicsComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity,
						Vec3 velocity,
						Vec3 acceleration,
						float engine_thrust,
						float mass,
						float wing_surface);

	void deleteComponent(Entity entity);

	void update(float timestep);

	const uint getIndex(Entity entity);

	void setEngineThrust(uint index, float engine_thrust);

	const Vec3 getVelocity(uint index);

	const Vec3 getAcceleration(uint index);
};
#endif