/// <copyright file="gltfAssetComponentManager.hpp">
/// Copyright © 2019 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>


#ifndef GLTF_ASSET_COMPONENT_MANAGER
#define GLTF_ASSET_COMPONENT_MANAGER

#include <string>
#include <vector>

#include "GenericVertexLayout.hpp"

namespace tinygltf
{
    class Model;
}

struct Entity;

namespace EngineCore
{
    class WorldState;

    namespace Graphics
    {
        template<typename ResourceManagerType>
        class GltfAssetComponentManager
        {
        public:
            GltfAssetComponentManager(ResourceManagerType& rsrc_mngr, WorldState& world);
            ~GltfAssetComponentManager() = default;

            void addComponent(Entity entity, std::string gltf_filepath, int gltf_node_idx);

            void importGltfScene(std::string gltf_filepath);

        private:
            struct GltfAsset
            {
                std::string     gltf_filepath;
                tinygltf::Model gltf_model; //TODO use unique pointer?
            };

            struct ComponentData
            {
                Entity entity;
                size_t gltf_asset_idx;
                int    gltf_node_idx;
            };

            std::vector<GltfAsset>     m_gltf_assets;
            std::vector<ComponentData> m_data;

            ResourceManagerType& m_rsrc_mngr;
            WorldState&          m_world;

            typedef std::shared_ptr<GenericVertexLayout>                     VertexLayoutPtr;
            typedef std::shared_ptr<std::vector<std::vector<unsigned char>>> VertexDataPtr;
            typedef std::shared_ptr<std::vector<unsigned char>>              IndexDataPtr;
            typedef unsigned int                                             IndexDataType;

            std::tuple<VertexLayoutPtr, VertexDataPtr, IndexDataPtr, IndexDataType>
                loadMeshPrimitveData(tinygltf::Model& model, size_t node_idx, size_t primitive_idx);
        };

        template<typename ResourceManagerType>
        inline GltfAssetComponentManager<ResourceManagerType>::GltfAssetComponentManager(ResourceManagerType & rsrc_mngr, WorldState & world)
        {
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addComponent(Entity entity, std::string gltf_filepath, int gltf_node_idx)
        {
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::importGltfScene(std::string gltf_filepath)
        {
        }


        template<typename ResourceManagerType>
        inline std::tuple<
            std::shared_ptr<GenericVertexLayout>,
            std::shared_ptr<std::vector<std::vector<unsigned char>>>,
            std::shared_ptr<std::vector<unsigned char>>,
            unsigned int> 
            GltfAssetComponentManager<ResourceManagerType>::loadMeshPrimitveData(tinygltf::Model & model, size_t node_idx, size_t primitive_idx)
        {
            return std::tuple<VertexLayoutPtr, VertexDataPtr, IndexDataPtr, IndexDataType>();
        }
    }
}

#endif // !GLTF_ASSET_COMPONENT_MANAGER
