#include "RearSteerBicycleComponent.hpp"

#include "utility.hpp"
#include "ResourceLoading.hpp"

void RearSteerBicycleComponentManager::addComponent(Entity e)
{
	std::unique_lock<std::shared_mutex> lock(m_data_mutex);

	// TODO check if entity already has transform component?

	uint idx = m_data.size();
	m_index_map.insert(std::pair<uint, uint>(e.id(), idx));

	Entity body_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(body_entity, Vec3(0.0, 0.0f, 0.0f), Quat(), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(body_entity), e);
	GRenderingComponents::staticMeshManager().addComponent(body_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/main_body.fbx");

	Entity handlebar_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(handlebar_entity, Vec3(0.0, 0.5f, 0.315f), Quat(), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(handlebar_entity), e);
	//GRenderingComponents::staticMeshManager().addComponent(handlebar_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/handlebar.fbx");

	Entity pedals_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(pedals_entity, Vec3(0.0, 0.0f, 0.0f), Quat(), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(pedals_entity), e);
	GRenderingComponents::staticMeshManager().addComponent(pedals_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/pedals.fbx");

	Entity rear_wheel_frame_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(rear_wheel_frame_entity, Vec3(0.0, 0.42f, 0.37f), glm::rotate(Quat(), (-10.0f)*(3.14f/180.0f),Vec3(1.0,0.0,0.0)), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(rear_wheel_frame_entity), e);
	GRenderingComponents::staticMeshManager().addComponent(rear_wheel_frame_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/rear_wheel_frame.fbx");

	Entity front_wheel_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(front_wheel_entity, Vec3(0.0, 0.01f, -0.5f), Quat(), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(front_wheel_entity), e);
	GRenderingComponents::staticMeshManager().addComponent(front_wheel_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/front_wheel.fbx");

	Entity rear_wheel_entity = GEngineCore::entityManager().create();
	GCoreComponents::transformManager().addComponent(rear_wheel_entity, Vec3(0.0, -0.45f, 0.16f), Quat(), Vec3(1.0f));
	GCoreComponents::transformManager().setParent(GCoreComponents::transformManager().getIndex(rear_wheel_entity), rear_wheel_frame_entity);
	GRenderingComponents::staticMeshManager().addComponent(rear_wheel_entity, "../resources/materials/dfr_debug.slmtl", "../resources/meshes/bicycle/rear_wheel.fbx");
	

	m_data.emplace_back(e);

	m_data[idx].m_body = body_entity;
	m_data[idx].m_handlebar = handlebar_entity;
	m_data[idx].m_pedals = pedals_entity;
	m_data[idx].m_rear_wheel_frame = rear_wheel_frame_entity;
	m_data[idx].m_front_wheel = front_wheel_entity;
	m_data[idx].m_rear_wheel = rear_wheel_entity;
}

void RearSteerBicycleComponentManager::addComponent(Entity e, std::string const& sim_data_path)
{
	addComponent(e);

	std::unique_lock<std::shared_mutex> lock(m_data_mutex);
	
	ResourceLoading::loadBikeSimulationData(sim_data_path, m_data.back().m_simulation_data);
}

void RearSteerBicycleComponentManager::update(float dt)
{
	std::shared_lock<std::shared_mutex> lock(m_data_mutex);

	uint idx = 0;
	for (auto& component : m_data)
	{
		//TODO update simulation state from data transfer or simulation code?
		SimulationState state = computeCurrentState(idx, dt);

		//TODO update body and rearwheel transform
		uint transform_idx = GCoreComponents::transformManager().getIndex(component.m_entity);
		uint rear_frame_transform_idx = GCoreComponents::transformManager().getIndex(component.m_rear_wheel_frame);
		uint handlebar_transform_idx = GCoreComponents::transformManager().getIndex(component.m_handlebar);

		uint pedal_transform_idx = GCoreComponents::transformManager().getIndex(component.m_pedals);
		uint rear_wheel_transform_idx = GCoreComponents::transformManager().getIndex(component.m_rear_wheel);
		uint front_wheel_transform_idx = GCoreComponents::transformManager().getIndex(component.m_front_wheel);

		GCoreComponents::transformManager().setPosition(transform_idx, Vec3(state.y_e,0.0f,state.x_e) );
		GCoreComponents::transformManager().setOrientation(transform_idx, glm::angleAxis(state.psi,Vec3(0.0,1.0,0.0)) * glm::angleAxis(-state.phi, Vec3(0.0, 0.0, 1.0)));

		Quat steer_orientation = glm::rotate(glm::rotate(Quat(), glm::radians(-10.0f), Vec3(1.0, 0.0, 0.0)), state.delta, Vec3(0.0f, 1.0f, 0.0f));
		GCoreComponents::transformManager().setOrientation(rear_frame_transform_idx, steer_orientation);
		GCoreComponents::transformManager().setOrientation(handlebar_transform_idx, steer_orientation);

		GCoreComponents::transformManager().rotate(pedal_transform_idx, glm::angleAxis(-state.omega * dt, Vec3(1.0f, 0.0f, 0.0f)));
		GCoreComponents::transformManager().rotate(rear_wheel_transform_idx, glm::angleAxis(-state.omega * dt, Vec3(1.0f, 0.0f, 0.0f)));
		GCoreComponents::transformManager().rotate(front_wheel_transform_idx, glm::angleAxis(-state.omega * dt, Vec3(1.0f, 0.0f, 0.0f)));

		++idx;
	}
}

SimulationState RearSteerBicycleComponentManager::computeCurrentState(uint idx, float timestep)
{
	m_data[idx].m_current_time += timestep;

	for (int i = 1; i < m_data[idx].m_simulation_data.size(); ++i)
	{
		if (m_data[idx].m_simulation_data[i].timestep > m_data[idx].m_current_time)
		{
			SimulationState interpolated_state;

			float timestep_length = 0.2;
			float alpha = (m_data[idx].m_current_time - m_data[idx].m_simulation_data[i - 1].timestep) / timestep_length;
			float beta = (m_data[idx].m_simulation_data[i].timestep - m_data[idx].m_current_time)/ timestep_length;

			interpolated_state.y_e = m_data[idx].m_simulation_data[i].y_e * alpha + m_data[idx].m_simulation_data[i - 1].y_e * beta;
			interpolated_state.x_e = m_data[idx].m_simulation_data[i].x_e * alpha + m_data[idx].m_simulation_data[i - 1].x_e * beta;
			interpolated_state.phi = m_data[idx].m_simulation_data[i].phi * alpha + m_data[idx].m_simulation_data[i - 1].phi * beta;
			interpolated_state.delta = m_data[idx].m_simulation_data[i].delta * alpha + m_data[idx].m_simulation_data[i - 1].delta * beta;
			interpolated_state.psi = m_data[idx].m_simulation_data[i].psi * alpha + m_data[idx].m_simulation_data[i - 1].psi * beta;
			interpolated_state.omega = m_data[idx].m_simulation_data[i].omega * alpha + m_data[idx].m_simulation_data[i - 1].omega * beta;

			return interpolated_state;
		}
	}

	m_data[idx].m_current_time = 0.0;

	return SimulationState();
}

void RearSteerBicycleComponentManager::pushSimulationState(uint idx, SimulationState state)
{
	m_data[idx].m_simulation_data[0] = state;
}

std::pair<bool, uint> RearSteerBicycleComponentManager::getIndex(Entity e) const
{
	return getIndex(e.id());
}

std::pair<bool, uint> RearSteerBicycleComponentManager::getIndex(uint eID) const
{
	return utility::entityToIndex(eID, m_index_map);
}

Entity RearSteerBicycleComponentManager::getEntity(uint idx) const
{
	return m_data[idx].m_entity;
}
