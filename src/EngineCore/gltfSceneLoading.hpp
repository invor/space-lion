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

namespace EngineCore
{
	struct Entity;
	class WorldState;

	namespace Graphics
	{
		namespace Dx11{
			struct VertexDescriptor;
		}
	}

	namespace Utility
	{
		namespace ResourceLoading
		{
			namespace Dx11
			{
				typedef std::shared_ptr<EngineCore::Graphics::Dx11::VertexDescriptor> VertexDescriptorPtr;
				typedef std::shared_ptr<std::vector<std::vector<unsigned char>>>      VertexDataPtr;
				typedef std::shared_ptr<std::vector<unsigned char>>                   IndexDataPtr;
				typedef int                                                           IndexDataType;

				std::tuple<VertexDescriptorPtr,VertexDataPtr,IndexDataPtr,IndexDataType> loadSingleNodeGLTFMeshPrimitiveData(std::shared_ptr<tinygltf::Model> model, size_t node_index, size_t primitive_idx);
			}

			std::shared_ptr<tinygltf::Model> loadGLTFModel(std::vector<unsigned char> const& databuffer);
			
			void addNode(int node_idx, Entity parent_node, std::shared_ptr<tinygltf::Model> const model, WorldState& world);

			std::future<void> loadScene(std::wstring gltf_filepath, WorldState& world);//TODO callback for adding a gltf node?
			
		}
	}
}

#endif // !gltfSceneLoading_hpp
