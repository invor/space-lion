#ifndef gltfAssetSystems_hpp
#define gltfAssetSystems_hpp

#include "BaseResourceManager.hpp"
#include "EntityManager.hpp"
#include "gltfAssetComponentManager.hpp"
#include "NameComponentManager.hpp"
#include "TransformComponentManager.hpp"
#include "WorldState.hpp"

#include "tiny_gltf.h"

namespace EngineCore
{
    namespace Graphics
    {
        namespace {
            template<typename ResourceManagerType>
            inline void addGltfNode(
                EngineCore::WorldState& world_state,
                ResourceManagerType& resource_manager,
                std::shared_ptr<tinygltf::Model> const& model,
                int gltf_node_idx,
                Entity parent_entity,
                std::unordered_map<int, Entity>& node_to_entity,
                ResourceID dflt_shader_prgm)
            {
                auto& gltf_asset_mngr = world_state.get<EngineCore::Graphics::GltfAssetComponentManager>();
                auto& name_mngr = world_state.get<EngineCore::Common::NameComponentManager>();
                auto& transform_mngr = world_state.get<EngineCore::Common::TransformComponentManager>();

                // add entity
                auto entity = m_world.accessEntityManager().create();
                node_to_entity.insert({ gltf_node_idx,entity });

                // add name
                name_mngr.addComponent(entity, model->nodes[gltf_node_idx].name);

                // add transform
                auto transform_idx = transform_mngr.addComponent(entity);

                if (parent_entity != m_world.accessEntityManager().invalidEntity())
                {
                    transform_mngr.setParent(transform_idx, parent_entity);
                }

                // traverse children and add gltf nodes recursivly
                for (auto child : gltf_model->nodes[gltf_node_idx].children)
                {
                    addGltfNode(gltf_model, child, entity, node_to_entity, dflt_shader_prgm);
                }

                // add gltf asset component (which in turn add mesh + material)
                addComponent(entity, gltf_model, gltf_node_idx, node_to_entity, dflt_shader_prgm);
            }
        }

        template<typename ResourceManagerType>
        inline std::vector<Entity> importGltfScene(
            EngineCore::WorldState& world_state,
            ResourceManagerType& resource_manager,
            std::string const& gltf_filepath,
            ResourceID dflt_shader_prgm)
        {
            auto& gltf_asset_mngr = world_state.get<GltfAssetComponentManager>();

            auto gltf_model = gltf_asset_mngr.addGltfAsset(gltf_filepath);

            // helper data structure for tracking entities that are created while traversing gltf scene graph
            std::unordered_map<int, Entity> node_to_entity;

            // iterate over nodes of model and add entities+components to world
            for (auto& scene : gltf_model->scenes)
            {
                for (auto node : scene.nodes)
                {
                    addGltfNode<ResourceManagerType>(
                        world_state,
                        resource_manager,
                        gltf_model,
                        node,
                        m_world.accessEntityManager().invalidEntity(),
                        node_to_entity,
                        dflt_shader_prgm
                    );
                }
            }
        }

        std::vector<Entity> importGltfNode();
    }
}

#endif // !gltfAssetSystems_hpp
