/// <copyright file="gltfSceneLoading.hpp">
/// Copyright © 2019 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef gltfLoading_hpp
#define gltfLoading_hpp

#include <future>

namespace tinygltf
{
    class Model;
}

struct VertexLayout;

namespace EngineCore
{
    struct Entity;
    class WorldState;

    namespace Graphics
    {
        namespace OpenGL
        {
            namespace ResourceLoading
            {
                typedef std::shared_ptr<VertexLayout> VertexDescriptorPtr;
                typedef std::shared_ptr<std::vector<std::vector<unsigned char>>> VertexDataPtr;
                typedef std::shared_ptr<std::vector<unsigned char>>              IndexDataPtr;
                typedef int                                                      IndexDataType;

                std::tuple<
                    VertexDescriptorPtr,
                    VertexDataPtr,
                    IndexDataPtr,
                    IndexDataType> loadSingleNodeGLTFMeshPrimitiveData(
                        std::shared_ptr<tinygltf::Model> model,
                        size_t node_index,
                        size_t primitive_idx
                    );
            }
        }
    }
}

#endif // !gltfLoading_hpp
