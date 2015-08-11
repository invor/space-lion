#ifndef TransformComponent_h
#define TransformComponent_h

#include "EntityManager.h"
#include "glm\mat4x4.hpp"
#include "glm\vec3.hpp"
#include "glm\gtc\quaternion.hpp"

typedef glm::mat4x4 Mat4x4;
typedef glm::vec3 Vec3;
typedef glm::quat Quat;


class TransformComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of transform components currently in use
		uint allocated;	///< number of transform components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning that owns the transform component
		Mat4x4* world_transform;	///< the actual transformation (aka model matrix)
		Vec3 position;
		Quat orientation;
		Vec3 scale;
	};

	Data m_data;

	void transform();

public:

	TransformComponentManager();
	TransformComponentManager(uint bytes);

	void reallocate();

	void addComponent(Entity entity);

	void deleteComonent(Entity entity);

	uint getIndex(Entity entity);

	void move(uint index);

	void rotate(uint index);

	void scale(Entity index);
};

#endif