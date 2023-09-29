/// <copyright file="gltfAssetComponentManager.hpp">
/// Copyright © 2019 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>


#ifndef GLTF_ASSET_COMPONENT_MANAGER
#define GLTF_ASSET_COMPONENT_MANAGER

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiny_gltf.h"

#include "BaseMultiInstanceComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "GenericVertexLayout.hpp"
#include "GenericTextureLayout.hpp"
#include "GeometryBakery.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "SkinComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"
#include "TransformComponentManager.hpp"

struct Entity;

namespace EngineCore
{
    class WorldState;

    namespace Graphics
    {
        namespace Utility
        {
            std::shared_ptr<tinygltf::Model> loadGltfModel(std::string const& gltf_filepath);

            std::shared_ptr<tinygltf::Model> loadGLTFModel(std::vector<unsigned char> const& databuffer);
        }

        class GltfAssetComponentManager : public BaseMultiInstanceComponentManager
        {
            struct ComponentData
            {
                Entity      entity;
                std::string gltf_asset_filepath;
                size_t      gltf_node_idx;
            };

        public:
            typedef std::shared_ptr<tinygltf::Model> ModelPtr;

            GltfAssetComponentManager() = default;
            ~GltfAssetComponentManager() = default;

            void addComponent(Entity entity, std::string const& gltf_filepath, size_t gltf_node_idx);

            ModelPtr addGltfModelToCache(std::string const& gltf_filepath);

            void addGltfModelToCache(std::string const& gltf_filepath, ModelPtr const& gltf_model);

            void clearModelCache();

            std::vector<ComponentData> getComponents() const;

        private:
            std::unordered_map<std::string, ModelPtr> m_gltf_models;
            std::shared_mutex                         m_gltf_models_mutex;
            std::vector<ComponentData>                m_data;
            mutable std::shared_mutex                 m_data_mutex;
        };
    }
}

#endif // !GLTF_ASSET_COMPONENT_MANAGER
