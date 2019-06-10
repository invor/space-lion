/// <copyright file="gltfAssetComponentManager.hpp">
/// Copyright © 2019 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>


#ifndef GLTF_ASSET_COMPONENT_MANAGER
#define GLTF_ASSET_COMPONENT_MANAGER

#include <string>

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
        template<typename ResourceManagerType, typename VertexLayoutType>
        class GltfAssetComponentManager
        {
        public:
            GltfAssetComponentManager(ResourceManagerType& rsrc_mngr, WorldState& world);
            ~GltfAssetComponentManager();

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



            ResourceManagerType& m_rsrc_mngr;
            WorldState&          m_world;


        };
    }
}

#endif // !GLTF_ASSET_COMPONENT_MANAGER
