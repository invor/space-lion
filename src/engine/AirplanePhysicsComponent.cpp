#include "AirplanePhysicsComponent.hpp"

AirplanePhysicsComponentManager::AirplanePhysicsComponentManager(uint size)
{
	const uint bytes = size * ( sizeof(Entity) +
								2*sizeof(Vec3) +
								5*sizeof(float) );

	m_data.buffer = new char[bytes];

	m_data.used = 0;
	m_data.allocated = size;

	m_data.entity = (Entity*)(m_data.buffer);
	m_data.velocity = (Vec3*)(m_data.entity + size);
	m_data.acceleration = (Vec3*)(m_data.velocity + size);
	m_data.engine_thrust = (float*)(m_data.acceleration + size);
	m_data.aerodynamic_drag = (float*)(m_data.engine_thrust + size);
	m_data.aerodynamic_lift = (float*)(m_data.aerodynamic_drag + size);
	m_data.wing_surface = (float*)(m_data.aerodynamic_lift + size);
	m_data.mass = (float*)(m_data.wing_surface + size);
}

AirplanePhysicsComponentManager::~AirplanePhysicsComponentManager()
{
	delete m_data.buffer;
}

void AirplanePhysicsComponentManager::reallocate(uint size)
{
	Data new_data;
	const uint bytes = size * ( sizeof(Entity) +
								2*sizeof(Vec3) +
								5*sizeof(float) );

	new_data.buffer = new char[bytes];

	new_data.used = m_data.used;
	new_data.allocated = size;

	new_data.entity = (Entity*)(new_data.buffer);
	new_data.velocity = (Vec3*)(new_data.entity + size);
	new_data.acceleration = (Vec3*)(new_data.velocity + size);
	new_data.engine_thrust = (float*)(new_data.acceleration + size);
	new_data.aerodynamic_drag = (float*)(new_data.engine_thrust + size);
	new_data.aerodynamic_lift = (float*)(new_data.aerodynamic_drag + size);
	new_data.wing_surface = (float*)(new_data.aerodynamic_lift + size);
	new_data.mass = (float*)(new_data.wing_surface + size);

	std::memcpy(new_data.entity, m_data.entity, m_data.used * sizeof(Entity));
	std::memcpy(new_data.velocity, m_data.velocity, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.acceleration, m_data.acceleration, m_data.used * sizeof(Vec3));
	std::memcpy(new_data.engine_thrust, m_data.engine_thrust, m_data.used * sizeof(float));
	std::memcpy(new_data.aerodynamic_drag, m_data.aerodynamic_drag, m_data.used * sizeof(float));
	std::memcpy(new_data.aerodynamic_lift, m_data.aerodynamic_lift, m_data.used * sizeof(float));
	std::memcpy(new_data.wing_surface, m_data.wing_surface, m_data.used * sizeof(float));
	std::memcpy(new_data.mass, m_data.mass, m_data.used * sizeof(float));

	delete m_data.buffer;

	m_data = new_data;
}

void AirplanePhysicsComponentManager::connectToTransformComponents(TransformComponentManager* transform_mngr)
{
	m_transform_mngr = transform_mngr;
}

void AirplanePhysicsComponentManager::addComponent(Entity entity,
													Vec3 velocity,
													Vec3 acceleration,
													float engine_thrust,
													float mass,
													float wing_surface)
{
	assert(m_data.used < m_data.allocated);

	uint index = m_data.used;

	m_index_map.insert({entity.id(),index});

	m_data.entity[index] = entity;
	m_data.velocity[index] = velocity;
	m_data.acceleration[index] = acceleration;
	m_data.engine_thrust[index] = engine_thrust;
	m_data.wing_surface[index] = wing_surface;
	m_data.mass[index] = mass;

	m_data.used++;

	// TODO update stuff
}

void AirplanePhysicsComponentManager::update(float timestep)
{
	// build coordinate frame of airplane (in world space)

	for(int i=0; i<m_data.used; i++)
	{

		uint transform_idx = m_transform_mngr->getIndex( m_data.entity[i] );
		Quat orientation = m_transform_mngr->getOrientation(transform_idx);

		Quat qFront = glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,0.0,1.0)),glm::conjugate(orientation));
		Quat qUp =	glm::cross(glm::cross(orientation,glm::quat(0.0,0.0,1.0,0.0)),glm::conjugate(orientation));
		Quat qRighthand = glm::cross(glm::cross(orientation,glm::quat(0.0,1.0,0.0,0.0)),glm::conjugate(orientation));

		// use the lift equation to compute updated aerodynamic lift:
		// lift = cl * (rho * v^2)/2 * A
		// and the drag equation to compute updated aerodynamic drag:
		// drag = 0.5 * rho * v^2 * A * cd

		float cl = 0.6f; // TODO: set based on angle of attack
		float cd = 0.05; // TODO: set based on angle of attack
		float rho = 1.2f; // TODO: depends on altitude
		float A = m_data.wing_surface[i];
		float v = std::sqrt( m_data.velocity[i].x*m_data.velocity[i].x + m_data.velocity[i].y*m_data.velocity[i].y + m_data.velocity[i].z*m_data.velocity[i].z);
		float m = m_data.mass[i];
		float g = 9.81;

		float angle_of_attack = glm::dot(Vec3(qFront.x,qFront.y,qFront.z),m_data.velocity[i]/v);

		m_data.aerodynamic_lift[i] = cl * ((rho * v*v)/2.0f) * A;

		m_data.aerodynamic_drag[i] = cd * ((rho * v*v)/2.0f) * A;

		m_data.acceleration[i] = ( g * Vec3(0.0f,-1.0f,0.0f) )
									+ ( (m_data.aerodynamic_lift[i]/m) * glm::cross(m_data.velocity[i]/v,Vec3(qRighthand.x,qRighthand.y,qRighthand.z)) )
									+ ( (m_data.engine_thrust[i]/m) * Vec3(qFront.x,qFront.y,qFront.z) )
									+ ( (m_data.aerodynamic_drag[i]/m) * -m_data.velocity[i]/v);

		std::cout<<"Drag Acceleration: "
					<<( (m_data.aerodynamic_drag[i]/m) * -m_data.velocity[i]/v).x<<" "
					<<( (m_data.aerodynamic_drag[i]/m) * -m_data.velocity[i]/v).y<<" "
					<<( (m_data.aerodynamic_drag[i]/m) * -m_data.velocity[i]/v).z<<std::endl;

		m_data.velocity[i] += m_data.acceleration[i] * timestep;

		m_transform_mngr->translate(transform_idx,m_data.velocity[i] * timestep);	
	}


}

const uint AirplanePhysicsComponentManager::getIndex(Entity entity)
{
}

void AirplanePhysicsComponentManager::setEngineThrust(uint index, float engine_thrust)
{
}

const Vec3 AirplanePhysicsComponentManager::getVelocity(uint index)
{
}

const Vec3 AirplanePhysicsComponentManager::getAcceleration(uint index)
{
}

