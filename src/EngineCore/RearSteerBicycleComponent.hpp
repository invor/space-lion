#ifndef RearSteerBicycleComponent_hpp
#define RearSteerBicycleComponent_hpp

#include <vector>
#include <unordered_map>

#include "GlobalEngineCore.hpp"
#include "EntityManager.hpp"

#include "GlobalCoreComponents.hpp"
#include "TransformComponent.hpp"

#include "GlobalRenderingComponents.hpp"
#include "StaticMeshComponent.hpp"

struct SimulationState
{
	SimulationState()
		: timestep(0.0f),
		x_e(0.0f),
		y_e(0.0f),
		phi(0.0f),
		phi_dt(0.0f),
		delta(0.0f),
		delta_dt(0.0f),
		psi(0.0f)
	{}

	float timestep;	///< Simulation timestep
	float x_e;		///< Position x
	float y_e;		///< Position y
	float phi;		///< Roll angle
	float phi_dt;	///< Roll speed
	float delta;	///< Steering angle
	float delta_dt; ///< Steering speed
	float psi;		///< Yaw angle
	float omega;	///< Wheel rotation speed
};

class RearSteerBicycleComponentManager
{

private:

	struct Data
	{
		Data(Entity e) 
			: m_entity(e),
			m_body(GEngineCore::entityManager().invalidEntity()),
			m_handlebar(GEngineCore::entityManager().invalidEntity()),
			m_pedals(GEngineCore::entityManager().invalidEntity()),
			m_rear_wheel_frame(GEngineCore::entityManager().invalidEntity()),
			m_front_wheel(GEngineCore::entityManager().invalidEntity()),
			m_rear_wheel(GEngineCore::entityManager().invalidEntity()) {}

		Entity m_entity;
		
		// Store all moving parts as sub entities so that each has it's own transform
		Entity m_body;
		Entity m_handlebar;
		Entity m_pedals;
		Entity m_rear_wheel_frame;
		Entity m_front_wheel;
		Entity m_rear_wheel;

		double m_current_time;
		SimulationState m_currentstate;

		std::vector<SimulationState> m_simulation_data;
	};

	std::vector<Data> m_data;
	mutable std::shared_mutex m_data_mutex;

	/** Map for fast entity to component index conversion */
	std::unordered_map<uint, uint> m_index_map;

public:

	void addComponent(Entity e);

	void addComponent(Entity e, std::string const& sim_data_path);

	void update(float dt);

	SimulationState computeCurrentState(uint idx, float timestep);

	void pushSimulationState(uint idx, SimulationState state);

	std::pair<bool, uint> getIndex(Entity e) const;

	std::pair<bool, uint> getIndex(uint eID) const;

	Entity getEntity(uint idx) const;
};


#endif