#ifndef TransformComponent_h
#define TransformComponent_h

// space-lion includes
#include "EntityManager.hpp"
#include "types.hpp"

// std includes
#include <unordered_map>
#include <iostream>
#include <shared_mutex>

namespace EngineCore
{
	namespace Common
	{
		class TransformComponentManager
		{
		private:
			struct Data
			{
				uint used;					///< number of components currently in use
				uint allocated;				///< number of components that the allocated memery can hold
				uint8_t* buffer;			///< raw data pointer

				Entity* entity;				///< entity that owns the component
				Mat4x4* world_transform;	///< the actual transformation (aka model matrix)
				Vec3* position;				///< local position (equals global position if component has no parent)
				Quat* orientation;			///< local orientation (...)
				Vec3* scale;				///< local scale (...)

				uint* parent;				///< index to parent (equals components own index if comp. has no parent)
				uint* first_child;			///< index to child (...)
				uint* next_sibling;			///< index to sibling (...)
			};

			Data m_data;

			std::unordered_map<uint, uint> m_index_map;
			mutable std::shared_mutex m_index_map_mutex;

			void transform(uint index);

		public:
			TransformComponentManager();
			TransformComponentManager(uint size);
			~TransformComponentManager();

			void reallocate(uint size);

			void addComponent(Entity entity, Vec3 position = Vec3(), Quat orientation = Quat(), Vec3 scale = Vec3(1.0));

			void deleteComonent(Entity entity);

			bool checkComponent(uint entity_id) const;

			uint getComponentCount() const;

			uint getIndex(Entity entity) const;

			uint getIndex(uint entity_id) const;

			void translate(Entity entity, Vec3 translation);

			void translate(uint index, Vec3 translation);

			void rotate(uint index, Quat rotation);

			void rotateLocal(uint index, Quat rotation);

			void scale(uint index, Vec3 scale_factors);

			void setPosition(Entity entity, Vec3 position);

			void setPosition(uint index, Vec3 position);

			void setOrientation(uint index, Quat orientation);

			void setParent(uint index, Entity parent);

			const Vec3 & getPosition(uint index) const;

			Vec3 getWorldPosition(uint index) const;

			Vec3 getWorldPosition(Entity e) const;

			const Quat & getOrientation(uint index) const;

			const Mat4x4 & getWorldTransformation(uint index) const;

			Mat4x4 const * const getWorldTransformations() const;
		};
	}
}

#endif