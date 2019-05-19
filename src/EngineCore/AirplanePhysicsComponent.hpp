#ifndef AirplanePhysicsComponent_hpp
#define AirplanePhysicsComponent_hpp

#include "BaseComponentManager.hpp"
#include "types.hpp"

struct Entity;

namespace EngineCore
{
    class WorldState;

    namespace Physics
    {

        class AirplanePhysicsComponentManager : public BaseComponentManager
        {
        private:

            struct AerodynamicForces
            {
                Vec3 lift;
                Vec3 drag;
                Vec3 thrust;
                Vec3 gravity;
            };

            struct Data
            {
                uint used;					///< number of components currently in use
                uint allocated;				///< number of components that the allocated memery can hold
                uint8_t* buffer;				///< raw data pointer

                Entity* entity;				///< entity owning the component

                Vec3* velocity;				///< velocity of the airplane (direction and magnitude)
                Vec3* acceleration;			///< acceleration of the airplance

                float* engine_thrust;		///< engine thrust, force acting in the direction of airplane orientation
                float* elevator_angle;		///< angle of the elevator, controls aircraft pitch
                float* rudder_angle;		///< angle of the rudder, controls aircraft yaw
                float* aileron_angle;		///< angle of the ailerons, controls aircraft banking
                float* pitch_torque;
                float* roll_torque;
                float* yaw_torque;

                float* angle_of_attack;
                float* angle_of_sideslip;
                float* lift_coefficient;
                float* drag_coefficient;
                float* aerodynamic_drag;	///< aerodynamic drag, force acting in opposite direction of the velocity
                float* aerodynamic_lift;	///< aerodynamic lift, force acting upward from airplane orientation

                float* wing_surface;		///< airplane wing surface (necessary for computing lift)
                float* mass;				///< airplane mass (necessary for computing lift)

            };

            Data m_data;

            mutable std::mutex m_data_mutex;

            WorldState& m_world;

        public:
            AirplanePhysicsComponentManager(uint size, WorldState& world);
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

            std::pair<bool, uint> getIndex(uint entity_id) const;

            float getEngineThrust(uint index) const;

            void setEngineThrust(uint index, float engine_thrust);

            void setElevatorAngle(uint index, float elevator_angle);

            void setRudderAngle(uint index, float rudder_angle);

            void setAileronAngle(uint index, float aileron_angle);

            Vec3 getVelocity(uint index) const;
            Vec3 getAcceleration(uint index) const;
            float getAngleOfAttack(uint index) const;
            float getAngleOfSideslip(uint index) const;
            float getLiftCoefficient(uint index) const;
            float getDragCoefficient(uint index) const;
            float getAerodynamicDrag(uint index) const;
            float getAerodynamicLift(uint index) const;
            float getAileronAngle(uint index) const;
            float getRudderAngle(uint index) const;
            float getElevatorAngle(uint index) const;
            float getWingSurface(uint index) const;
        };

    }

}
#endif