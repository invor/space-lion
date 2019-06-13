/// <copyright file="gltfSceneLoading.hpp">
/// Copyright © 2019 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef gltfSceneLoading_hpp
#define gltfSceneLoading_hpp

#include <future>

namespace tinygltf
{
	class Model;
}

struct Entity;

struct GenericVertexLayout;

namespace EngineCore
{
	class WorldState;

	namespace Graphics
	{
		namespace OpenGL
		{
            class ResourceManager;

			typedef std::shared_ptr<GenericVertexLayout>                          VertexLayoutPtr;
			typedef std::shared_ptr<std::vector<std::vector<unsigned char>>>      VertexDataPtr;
			typedef std::shared_ptr<std::vector<unsigned char>>                   IndexDataPtr;
			typedef unsigned int                                                  IndexDataType;

			std::tuple<VertexLayoutPtr,VertexDataPtr,IndexDataPtr,IndexDataType> 
                loadSingleNodeGLTFMeshPrimitiveData(std::shared_ptr<tinygltf::Model> model, size_t node_index, size_t primitive_idx);
			
			std::shared_ptr<tinygltf::Model> loadGLTFModel(std::string gltf_filepath);
			
			void addGLTFNode(
                int node_idx,
                Entity parent_node,
                std::shared_ptr<tinygltf::Model> const model,
                WorldState& world,
                ResourceManager& resource_mngr);

			void loadGLTFScene(std::string gltf_filepath, WorldState& world, ResourceManager& resource_mngr);//TODO callback for adding a gltf node?
			
		}
	}
}

#endif // !gltfSceneLoading_hpp
