#ifndef TransformComponent_h
#define TransformComponent_h

#include <unordered_map>
#include <iostream>

#include "EntityManager.hpp"
#include "types.hpp"


class TransformComponentManager
{
private:
	struct Data
	{
		uint used;		///< number of components currently in use
		uint allocated;	///< number of components that the allocated memery can hold
		void* buffer;	///< raw data pointer

		Entity* entity;				///< entity owning that owns the component
		Mat4x4* world_transform;	///< the actual transformation (aka model matrix)
		Vec3* position;				///< local position (equals global position if component has no parent)
		Quat* orientation;			///< local orientation (...)
		Vec3* scale;				///< local scale (...)

		uint* parent;				///< index to parent (equals components own index if comp. has no parent)
		uint* first_child;			///< index to child (...)
		uint* next_sibling;			///< index to sibling (...)
	};

	Data m_data;

	std::unordered_map<uint,uint> m_index_map;

	void transform(uint index);

public:
	TransformComponentManager();
	TransformComponentManager(uint size);
	~TransformComponentManager();

	void reallocate(uint size);

	void addComponent(Entity entity);

	void addComponent(Entity entity, Vec3 position, Quat orientation, Vec3 scale);

	void deleteComonent(Entity entity);

	uint getIndex(Entity entity);

	void move(uint index, Vec3 translation);

	void rotate(uint index, Quat rotation);

	void scale(uint index, Vec3 scale_factors);

	const Vec3 getPosition(uint index);

	const Mat4x4 getWorldTransformation(uint index);

	//const Mat4x4* getWorldTransformations();
};

#endif