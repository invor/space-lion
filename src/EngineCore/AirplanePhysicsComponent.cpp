#include "AirplanePhysicsComponent.hpp"

#include "EntityManager.hpp"
#include "TransformComponentManager.hpp"
#include "WorldState.hpp"

namespace EngineCore
{
	namespace Physics
	{

		AirplanePhysicsComponentManager::AirplanePhysicsComponentManager(uint size, WorldState& world)
			: m_world(world)
		{
			const uint bytes = size * (sizeof(Entity) +
				2 * sizeof(Vec3) +
				15 * sizeof(float));

			m_data.buffer = new uint8_t[bytes];

			m_data.used = 0;
			m_data.allocated = size;

			m_data.entity = (Entity*)(m_data.buffer);
			m_data.velocity = (Vec3*)(m_data.entity + size);
			m_data.acceleration = (Vec3*)(m_data.velocity + size);
			m_data.engine_thrust = (float*)(m_data.acceleration + size);
			m_data.elevator_angle = (float*)(m_data.engine_thrust + size);
			m_data.rudder_angle = (float*)(m_data.elevator_angle + size);
			m_data.aileron_angle = (float*)(m_data.rudder_angle + size);

			m_data.pitch_torque = (float*)(m_data.aileron_angle + size);
			m_data.roll_torque = (float*)(m_data.pitch_torque + size);
			m_data.yaw_torque = (float*)(m_data.roll_torque + size);

			m_data.angle_of_attack = (float*)(m_data.yaw_torque + size);
			m_data.angle_of_sideslip = (float*)(m_data.angle_of_attack + size);
			m_data.lift_coefficient = (float*)(m_data.angle_of_sideslip + size);
			m_data.drag_coefficient = (float*)(m_data.lift_coefficient + size);
			m_data.aerodynamic_drag = (float*)(m_data.drag_coefficient + size);
			m_data.aerodynamic_lift = (float*)(m_data.aerodynamic_drag + size);
			m_data.wing_surface = (float*)(m_data.aerodynamic_lift + size);
			m_data.mass = (float*)(m_data.wing_surface + size);
		}

		AirplanePhysicsComponentManager::~AirplanePhysicsComponentManager()
		{
			delete[] m_data.buffer;
		}

		void AirplanePhysicsComponentManager::reallocate(uint size)
		{
			Data new_data;
			const uint bytes = size * (sizeof(Entity) +
				2 * sizeof(Vec3) +
				15 * sizeof(float));

			new_data.buffer = new uint8_t[bytes];

			new_data.used = m_data.used;
			new_data.allocated = size;

			new_data.entity = (Entity*)(new_data.buffer);
			new_data.velocity = (Vec3*)(new_data.entity + size);
			new_data.acceleration = (Vec3*)(new_data.velocity + size);
			new_data.engine_thrust = (float*)(new_data.acceleration + size);
			new_data.elevator_angle = (float*)(new_data.engine_thrust + size);
			new_data.rudder_angle = (float*)(new_data.elevator_angle + size);
			new_data.aileron_angle = (float*)(new_data.rudder_angle + size);
			new_data.pitch_torque = (float*)(new_data.aileron_angle + size);
			new_data.roll_torque = (float*)(new_data.pitch_torque + size);
			new_data.yaw_torque = (float*)(new_data.roll_torque + size);
			new_data.angle_of_attack = (float*)(new_data.yaw_torque + size);
			new_data.angle_of_sideslip = (float*)(new_data.angle_of_attack + size);
			new_data.lift_coefficient = (float*)(new_data.angle_of_sideslip + size);
			new_data.drag_coefficient = (float*)(new_data.lift_coefficient + size);
			new_data.aerodynamic_drag = (float*)(new_data.aileron_angle + size);
			new_data.aerodynamic_lift = (float*)(new_data.aerodynamic_drag + size);
			new_data.wing_surface = (float*)(new_data.aerodynamic_lift + size);
			new_data.mass = (float*)(new_data.wing_surface + size);

			std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
			std::memcpy(new_data.velocity, m_data.velocity, m_data.used * sizeof(Vec3));
			std::memcpy(new_data.acceleration, m_data.acceleration, m_data.used * sizeof(Vec3));
			std::memcpy(new_data.engine_thrust, m_data.engine_thrust, m_data.used * sizeof(float));
			std::memcpy(new_data.elevator_angle, m_data.elevator_angle, m_data.used * sizeof(float));
			std::memcpy(new_data.rudder_angle, m_data.rudder_angle, m_data.used * sizeof(float));
			std::memcpy(new_data.aileron_angle, m_data.aileron_angle, m_data.used * sizeof(float));

			std::memcpy(new_data.pitch_torque, m_data.pitch_torque, m_data.used * sizeof(float));
			std::memcpy(new_data.roll_torque, m_data.roll_torque, m_data.used * sizeof(float));
			std::memcpy(new_data.yaw_torque, m_data.yaw_torque, m_data.used * sizeof(float));

			std::memcpy(new_data.angle_of_attack, m_data.angle_of_attack, m_data.used * sizeof(float));
			std::memcpy(new_data.angle_of_sideslip, m_data.angle_of_sideslip, m_data.used * sizeof(float));
			std::memcpy(new_data.lift_coefficient, m_data.lift_coefficient, m_data.used * sizeof(float));
			std::memcpy(new_data.drag_coefficient, m_data.drag_coefficient, m_data.used * sizeof(float));
			std::memcpy(new_data.aerodynamic_drag, m_data.aerodynamic_drag, m_data.used * sizeof(float));
			std::memcpy(new_data.aerodynamic_lift, m_data.aerodynamic_lift, m_data.used * sizeof(float));
			std::memcpy(new_data.wing_surface, m_data.wing_surface, m_data.used * sizeof(float));
			std::memcpy(new_data.mass, m_data.mass, m_data.used * sizeof(float));

			delete m_data.buffer;

			m_data = new_data;
		}

		void AirplanePhysicsComponentManager::addComponent(
			Entity entity,
			Vec3 velocity,
			Vec3 acceleration,
			float engine_thrust,
			float mass,
			float wing_surface)
		{
			assert(m_data.used < m_data.allocated);

			uint index = m_data.used;

			addIndex(entity.id(), index);

			m_data.entity[index] = entity;
			m_data.velocity[index] = velocity;
			m_data.acceleration[index] = acceleration;
			m_data.engine_thrust[index] = engine_thrust;
			m_data.elevator_angle[index] = 0.0f;
			m_data.rudder_angle[index] = 0.0f;
			m_data.aileron_angle[index] = 0.0f;

			m_data.pitch_torque[index] = 0.0f;
			m_data.roll_torque[index] = 0.0f;
			m_data.yaw_torque[index] = 0.0f;

			m_data.angle_of_attack[index] = 0.0f;
			m_data.lift_coefficient[index] = 0.0f;
			m_data.drag_coefficient[index] = 0.0f;
			m_data.wing_surface[index] = wing_surface;
			m_data.mass[index] = mass;

			m_data.used++;

			// TODO update stuff
		}

		void AirplanePhysicsComponentManager::update(float timestep)
		{
			std::unique_lock<std::mutex> lock(m_data_mutex);

			for (uint i = 0; i < m_data.used; i++)
			{
				uint aircraft_transform_idx =  m_world.accessTransformManager().getIndex(m_data.entity[i]);
				Quat orientation = m_world.accessTransformManager().getOrientation(aircraft_transform_idx);

				Quat qFront = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 0.0, 1.0)), glm::conjugate(orientation));
				Quat qUp = glm::cross(glm::cross(orientation, glm::quat(0.0, 0.0, 1.0, 0.0)), glm::conjugate(orientation));
				Quat qRighthand = glm::cross(glm::cross(orientation, glm::quat(0.0, -1.0, 0.0, 0.0)), glm::conjugate(orientation));

				Vec3 vFront(qFront.x, qFront.y, qFront.z);
				Vec3 vUp(qUp.x, qUp.y, qUp.z);
				Vec3 vRighthand(qRighthand.x, qRighthand.y, qRighthand.z);

				// use the lift equation to compute updated aerodynamic lift:
				// lift = cl * (rho * v^2)/2 * A
				// and the drag equation to compute updated aerodynamic drag:
				// drag = 0.5 * rho * v^2 * A * cd

				float min_airbrake = 0.2f;
				float max_airbrake = 0.6f;

				float rho = 1.2f; // TODO: depends on altitude
				float A = m_data.wing_surface[i];
				float v = glm::length(m_data.velocity[i]);
				float m = m_data.mass[i];
				float g = 9.81f;

				// project velocity into aircraft yz plane
				Vec3 v_yz = glm::normalize(glm::normalize(m_data.velocity[i]) - (glm::dot(vRighthand, glm::normalize(m_data.velocity[i])) * vRighthand));
				Vec3 v_xz = glm::normalize(glm::normalize(m_data.velocity[i]) - (glm::dot(vUp, glm::normalize(m_data.velocity[i])) * vUp));
				float aoa = glm::dot(vFront, v_yz);
				float aos = glm::dot(vFront, v_xz);

				//transform angle of attack to degree
				aoa = (aoa < -1.0f) ? -1.0f : (aoa > 1.0f) ? 1.0f : aoa;
				aos = (aos < -1.0f) ? -1.0f : (aos > 1.0f) ? 1.0f : aos;

				aoa = (std::acos(aoa) / 3.14f) * 180.0f;
				aos = (std::acos(aos) / 3.14f) * 180.0f;

				// negative angle of attack!
				aoa = glm::dot(vUp, v_yz) > 0.0f ? -aoa : aoa;
				aos = glm::dot(vRighthand, v_xz) > 0.0f ? -aos : aos;

				m_data.angle_of_attack[i] = aoa;
				m_data.angle_of_sideslip[i] = aos;

				//float cl = (aoa < 16.0f) ? 0.0889*aoa + 0.178 : -0.1*aoa+3.2;
				float cl = (aoa < 16.0f) ? 0.1f*aoa : -0.1f*aoa + 3.2f;
				cl = cl / (1.0f + aos / 45.0f);
				//cl = (abs(aoa) > 25.0f) ? 0.0f : cl;
				m_data.lift_coefficient[i] = cl;

				float cdp = 0.034f;
				float eff = 0.77f;
				//float cd = cdp + cl*cl / (3.14f * (64.0f/22.0f) * eff);
				// modify induced drag to not decrease after passing the stall angle
				float cd = cdp + ((0.0889f*aoa + 0.178f)*(0.0889f*aoa + 0.178f)) / (3.14f * (49.0f / 22.0f) * eff)
					+ ((aos*aos) / 2000.0f);
				m_data.drag_coefficient[i] = cd;

				m_data.aerodynamic_lift[i] = cl * ((rho * v*v) / 2.0f) * A;
				m_data.aerodynamic_drag[i] = cd * ((rho * v*v) / 2.0f) * A;

				m_data.acceleration[i] = (g * Vec3(0.0f, -1.0f, 0.0f))
					+ ((m_data.aerodynamic_lift[i] / m) * vUp)
					+ ((m_data.engine_thrust[i] / m) * vFront)
					+ ((m_data.aerodynamic_drag[i] / m) * -glm::normalize(m_data.velocity[i]));

				m_data.velocity[i] += m_data.acceleration[i] * timestep;

				// based on rudder/elevator/aileron configuration, aoa and windspeed update aircraft orientations
				float elevator_angle = m_data.elevator_angle[i] * 1.0f;
				float rudder_angle = m_data.rudder_angle[i] * 1.0f;
				float aileron_angle = m_data.aileron_angle[i] * 1.0f;

				// orientation is updated in local coordinate frame
				//	orientation *= glm::rotate(glm::quat(), -0.02f * aileron_angle / (1.0f + abs(aoa)), Vec3(0.0, 0.0, -1.0)); // roll
				//	orientation *= glm::rotate(glm::quat(), -0.005f * elevator_angle / (1.0f + abs(aoa)), Vec3(1.0, 0.0, 0.0)); // pitch
				//	orientation *= glm::rotate(glm::quat(), 0.001f * rudder_angle / (1.0f + abs(aos)), Vec3(0.0, 1.0, 0.0)); // yaw

				// add rotation about center of mass induced by gravity and induced by lift...very, very simplified
				float gravity_torque = -0.000025f * m;
				float lift_torque = 0.000025f * m;


				Quat worldUp_aircraftSpace_q = glm::conjugate(orientation) * glm::quat(0.0, 0.0, 1.0, 0.0) * orientation;
				Vec3 worldUp_aircraftSpace = Vec3(worldUp_aircraftSpace_q.x, worldUp_aircraftSpace_q.y, worldUp_aircraftSpace_q.z);
				Quat velocity_aircraftSpace_q = glm::conjugate(orientation) * glm::quat(0.0, m_data.velocity[i]) * orientation;
				Vec3 velocity_aircraftSpace = Vec3(velocity_aircraftSpace_q.x, velocity_aircraftSpace_q.y, velocity_aircraftSpace_q.z);

				Vec3 aircraftSpace_worldX = glm::cross(velocity_aircraftSpace, worldUp_aircraftSpace);

				// rotations are applied in aircraft space!
				orientation *= glm::rotate(glm::quat(), timestep *gravity_torque, aircraftSpace_worldX);
				orientation *= glm::rotate(glm::quat(), timestep *lift_torque, Vec3(-1.0f, 0.0f, 0.0f));


				m_data.roll_torque[i] = (m_data.roll_torque[i] + ((aileron_angle / (1.0f + 0.5f*abs(aoa))) - 5.0f*m_data.roll_torque[i]) * timestep);
				m_data.pitch_torque[i] = (m_data.pitch_torque[i] + ((elevator_angle / (1.0f + 0.5f*abs(aoa))) - 5.0f*m_data.pitch_torque[i]) * timestep);
				m_data.yaw_torque[i] = (m_data.yaw_torque[i] + ((rudder_angle / (1.0f + 0.5f*abs(aos))) - 10.0f*m_data.yaw_torque[i]) * timestep);


				orientation *= glm::rotate(glm::quat(), timestep * 12.0f * (m_data.roll_torque[i]), Vec3(0.0, 0.0, 1.0)); // roll
				orientation *= glm::rotate(glm::quat(), timestep * 7.0f * (m_data.pitch_torque[i]), Vec3(-1.0, 0.0, 0.0)); // pitch
				orientation *= glm::rotate(glm::quat(), timestep * 3.0f * (m_data.yaw_torque[i]), Vec3(0.0, 1.0, 0.0)); // yaw

				m_world.accessTransformManager().setOrientation(aircraft_transform_idx, orientation);
				m_world.accessTransformManager().translate(aircraft_transform_idx, m_data.velocity[i] * timestep);
			}
		}

		float AirplanePhysicsComponentManager::getEngineThrust(uint index) const
		{
			assert(index < m_data.used);

			return m_data.engine_thrust[index];
		}

		void AirplanePhysicsComponentManager::setEngineThrust(uint index, float engine_thrust)
		{
			assert(index < m_data.used);

			m_data.engine_thrust[index] = engine_thrust;
		}

		Vec3 AirplanePhysicsComponentManager::getVelocity(uint index) const
		{
			assert(index < m_data.used);

			return m_data.velocity[index];
		}

		Vec3 AirplanePhysicsComponentManager::getAcceleration(uint index) const
		{
			assert(index < m_data.used);

			return m_data.acceleration[index];
		}

		void AirplanePhysicsComponentManager::setElevatorAngle(uint index, float elevator_angle)
		{
			std::unique_lock<std::mutex> lock(m_data_mutex);
			m_data.elevator_angle[index] = elevator_angle;
		}

		void AirplanePhysicsComponentManager::setRudderAngle(uint index, float rudder_angle)
		{
			std::unique_lock<std::mutex> lock(m_data_mutex);
			m_data.rudder_angle[index] = rudder_angle;
		}

		void AirplanePhysicsComponentManager::setAileronAngle(uint index, float aileron_angle)
		{
			std::unique_lock<std::mutex> lock(m_data_mutex);
			m_data.aileron_angle[index] = aileron_angle;
		}

		float AirplanePhysicsComponentManager::getAngleOfAttack(uint index) const
		{
			return m_data.angle_of_attack[index];
		}

		float AirplanePhysicsComponentManager::getAngleOfSideslip(uint index) const
		{
			return m_data.angle_of_sideslip[index];
		}

		float AirplanePhysicsComponentManager::getLiftCoefficient(uint index) const
		{
			return m_data.lift_coefficient[index];
		}

		float AirplanePhysicsComponentManager::getDragCoefficient(uint index) const
		{
			return m_data.drag_coefficient[index];
		}

		float AirplanePhysicsComponentManager::getAerodynamicDrag(uint index) const
		{
			return m_data.aerodynamic_drag[index];
		}

		float AirplanePhysicsComponentManager::getAerodynamicLift(uint index) const
		{
			return m_data.aerodynamic_lift[index];
		}

		float AirplanePhysicsComponentManager::getAileronAngle(uint index) const
		{
			return m_data.aileron_angle[index];
		}

		float AirplanePhysicsComponentManager::getRudderAngle(uint index) const
		{
			return m_data.rudder_angle[index];
		}

		float AirplanePhysicsComponentManager::getElevatorAngle(uint index) const
		{
			return m_data.elevator_angle[index];
		}

		float AirplanePhysicsComponentManager::getWingSurface(uint index) const
		{
			return m_data.wing_surface[index];
		}

	}
}