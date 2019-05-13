#ifndef PtexMeshComponent_hpp
#define PtexMeshComponent_hpp

#include <vector>
#include <unordered_map>

#include "EntityManager.hpp"
#include "OpenGL/ResourceManager.hpp"

struct Frame;
class GLSLProgram;
class Mesh;
class ShaderStorageBufferObject;

namespace EngineCore
{
	namespace Graphics
	{
		class PtexMeshComponentManager
		{
		private:
			struct Data
			{
				Data(Entity entity,
					ResourceID mesh,
					ResourceID material,
					ResourceID ptex_params,
					ResourceID ptex_bindless_texture_handles,
					bool visible)
					: entity(entity),
					mesh(mesh),
					material(material),
					ptex_parameters(ptex_params),
					ptex_bindless_texture_handles(ptex_bindless_texture_handles),
					visible(visible) {}

				Entity entity;
				ResourceID mesh;
				ResourceID material;
				ResourceID ptex_parameters;
				ResourceID ptex_bindless_texture_handles;
				bool visible;
			};

			std::vector<Data> m_data;

			std::unordered_map<uint, uint> m_index_map;

		public:
			/** Add component using existing GPU resources */
			void addComponent(Entity e, ResourceID mesh, ResourceID material, ResourceID ptex_parameters, ResourceID ptex_bindless_texture_handles, bool visible = true);

			std::pair<bool, uint> getIndex(Entity e) const;

			void setVisibility(Entity e, bool visible);

			void setPtexParameters(Entity e, ResourceID ptex_params);

			ResourceID getMesh(uint idx);

			ResourceID getMaterial(uint idx);

			ResourceID getPtexParameters(uint idx);

			ResourceID getPtexBindTextureHandles(uint idx);

			std::vector<Entity> getListOfEntities() const;
		};
	}
}

#endif // !PtexMeshComponent_hpp
