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

#include "tiny_gltf.h"

#include "BaseComponentManager.hpp"
#include "BaseResourceManager.hpp"
#include "GenericVertexLayout.hpp"
#include "GenericTextureLayout.hpp"

struct Entity;

namespace EngineCore
{
    class WorldState;

    namespace Graphics
    {
        namespace Utility
        {
            std::shared_ptr<tinygltf::Model> loadGltfModel(std::string const& gltf_filepath);

            std::shared_ptr<tinygltf::Model> loadGLTFModel(std::vector<unsigned char> const & databuffer);
        }

        template<typename ResourceManagerType>
        class GltfAssetComponentManager : public BaseComponentManager
        {
        public:
            GltfAssetComponentManager(ResourceManagerType& rsrc_mngr, WorldState& world);
            ~GltfAssetComponentManager() = default;

            void addComponent(Entity entity, std::string const& gltf_filepath, std::string const& gltf_node_name);

            void addComponent(Entity entity, std::string const& gltf_filepath, int gltf_node_idx);

            void importGltfScene(std::string const& gltf_filepath, ResourceID dflt_shader_prgm);

            void importGltfScene(std::string const& gltf_filepath, std::shared_ptr<tinygltf::Model> const& gltf_model, ResourceID dflt_shader_prgm);

        private:
            typedef std::shared_ptr<tinygltf::Model> ModelPtr;

            struct ComponentData
            {
                Entity   entity;
                ModelPtr gltf_asset_idx;
                int      gltf_node_idx;
            };

            std::unordered_map<std::string, ModelPtr> m_gltf_assets;
            std::shared_mutex                         m_gltf_assets_mutex;
            std::vector<ComponentData>                m_data;
            std::shared_mutex                         m_data_mutex;

            ResourceManagerType& m_rsrc_mngr;
            WorldState&          m_world;

            void addComponent(Entity entity, ModelPtr const& model, int gltf_node_idx, ResourceID dflt_shader_prgm);

            ModelPtr addGltfAsset(std::string const& gltf_filepath);

            void addGltfAsset(std::string const& gltf_filepath, ModelPtr const& gltf_model);

            void addGltfNode(ModelPtr const& model, int gltf_node_idx, Entity parent_entity, ResourceID dflt_shader_prgm);

            typedef std::shared_ptr<typename ResourceManagerType::VertexLayout> VertexLayoutPtr;
            typedef std::shared_ptr<std::vector<std::vector<unsigned char>>>    VertexDataPtr;
            typedef std::shared_ptr<std::vector<unsigned char>>                 IndexDataPtr;
            typedef typename ResourceManagerType::IndexFormatType               IndexDataType;
            //typedef unsigned int                                                IndexDataType;

            std::tuple<VertexLayoutPtr, VertexDataPtr, IndexDataPtr, IndexDataType>
                loadMeshPrimitveData(ModelPtr const& model, size_t node_idx, size_t primitive_idx);
        };

        template<typename ResourceManagerType>
        inline GltfAssetComponentManager<ResourceManagerType>::GltfAssetComponentManager(ResourceManagerType & rsrc_mngr, WorldState & world)
            : m_rsrc_mngr(rsrc_mngr), m_world(world)
        {
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addComponent(
            Entity entity,
            std::string const& gltf_filepath,
            std::string const& gltf_node_name)
        {
            ModelPtr model(nullptr);

            {
                std::shared_lock<std::shared_mutex> lock(m_gltf_assets_mutex);
                auto query = m_gltf_assets.find(gltf_filepath);
                if (query != m_gltf_assets.end())
                {
                    model = query->second;
                }
                else
                {
                    lock.unlock();
                    model = addGltfAsset(gltf_filepath);
                }
            }

            for (int i = 0; i < model->nodes.size(); ++i)
            {
                if (model->nodes[i].name == gltf_node_name)
                {
                    addComponent(entity, model, i);
                }
            }
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addComponent(
            Entity entity,
            std::string const& gltf_filepath,
            int gltf_node_idx)
        {
            ModelPtr model(nullptr);

            {
                std::shared_lock<std::shared_mutex> lock(m_gltf_assets_mutex);
                auto query = m_gltf_assets.find(gltf_filepath);
                if (query != m_gltf_assets.end())
                {
                    model = query->second;
                }
                else
                {
                    lock.unlock();
                    model = addGltfAsset(gltf_filepath);
                }
            }

            addComponent(entity, model, gltf_node_idx);
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::importGltfScene(std::string const& gltf_filepath, ResourceID dflt_shader_prgm)
        {
            auto gltf_model = addGltfAsset(gltf_filepath);

            // iterate over nodes of model and add entities+components to world
            for (auto& scene : gltf_model->scenes)
            {
                for (auto node : scene.nodes)
                {
                    addGltfNode(gltf_model, node, m_world.accessEntityManager().invalidEntity(), dflt_shader_prgm);
                }
            }
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::importGltfScene(std::string const& gltf_filepath, std::shared_ptr<tinygltf::Model> const & gltf_model, ResourceID dflt_shader_prgm)
        {
            addGltfAsset(gltf_filepath, gltf_model);

            // iterate over nodes of model and add entities+components to world
            for (auto& scene : gltf_model->scenes)
            {
                for (auto node : scene.nodes)
                {
                    addGltfNode(gltf_model, node, m_world.accessEntityManager().invalidEntity(), dflt_shader_prgm);
                }
            }
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addGltfNode(ModelPtr const & gltf_model, int gltf_node_idx, Entity parent_entity, ResourceID dflt_shader_prgm)
        {
            // add entity
            auto entity = m_world.accessEntityManager().create();

            // add name
            //m_world.accessNameManager()->addComponent(entity, model->nodes[node].name);

            // add transform
            auto transform_idx = m_world.accessTransformManager().addComponent(entity);

            if (parent_entity != m_world.accessEntityManager().invalidEntity())
            {
                m_world.accessTransformManager().setParent(transform_idx, parent_entity);
            }

            // add gltf asset component (which in turn add mesh + material)
            addComponent(entity, gltf_model, gltf_node_idx, dflt_shader_prgm);

            // traverse children and add gltf nodes recursivly
            for (auto child : gltf_model->nodes[gltf_node_idx].children)
            {
                addGltfNode(gltf_model, child, entity, dflt_shader_prgm);
            }
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addComponent(Entity entity, ModelPtr const& model, int gltf_node_idx, ResourceID dflt_shader_prgm)
        {
            {
                std::unique_lock<std::shared_mutex> lock(m_data_mutex);

                size_t cmp_idx = m_data.size();
                m_data.push_back({ entity,model,gltf_node_idx });
                addIndex(entity.id(), cmp_idx);
            }

            size_t transform_idx = m_world.accessTransformManager().getIndex(entity).front();

            if (model->nodes[gltf_node_idx].matrix.size() != 0) // has matrix transform
            {
                // TODO
            }
            else
            {
                auto& translation = model->nodes[gltf_node_idx].translation;
                auto& scale = model->nodes[gltf_node_idx].scale;
                auto& rotation = model->nodes[gltf_node_idx].rotation;

                if (translation.size() != 0) {
                    m_world.accessTransformManager().setPosition(
                        transform_idx,
                        Vec3(static_cast<float>(translation[0]),
                            static_cast<float>(translation[1]),
                            static_cast<float>(translation[2]))
                    );
                }
                if (scale.size() != 0) {
                    m_world.accessTransformManager().scale(
                        transform_idx,
                        Vec3(static_cast<float>(scale[0]),
                            static_cast<float>(scale[1]),
                            static_cast<float>(scale[2]))
                    );
                }
                if (rotation.size() != 0) {
                    m_world.accessTransformManager().setOrientation(
                        transform_idx,
                        Quat(static_cast<float>(rotation[0]),
                            static_cast<float>(rotation[1]),
                            static_cast<float>(rotation[2]),
                            static_cast<float>(rotation[3]))
                    );
                }
            }

            if (model->nodes[gltf_node_idx].mesh != -1)
            {
                auto primitive_cnt = model->meshes[model->nodes[gltf_node_idx].mesh].primitives.size();

                for (size_t primitive_idx = 0; primitive_idx < primitive_cnt; ++primitive_idx)
                {
                    // add bbox component
                    auto max_data = model->accessors[model->meshes[model->nodes[gltf_node_idx].mesh].primitives[primitive_idx].attributes.find("POSITION")->second].maxValues;
                    auto min_data = model->accessors[model->meshes[model->nodes[gltf_node_idx].mesh].primitives[primitive_idx].attributes.find("POSITION")->second].minValues;
                    Vec3 max(static_cast<float>(max_data[0]), static_cast<float>(max_data[1]), static_cast<float>(max_data[2]));
                    Vec3 min(static_cast<float>(min_data[0]), static_cast<float>(min_data[1]), static_cast<float>(min_data[2]));

                    //app_content.accessBoundingBoxManager().addComponent(entity, DirectX::BoundingBox(
                    //	(max + min) * 0.5f,
                    //	(max - min) * 0.5f));

                    auto mesh_data = loadMeshPrimitveData(model, gltf_node_idx, primitive_idx);

                    auto material_idx = model->meshes[model->nodes[gltf_node_idx].mesh].primitives[primitive_idx].material;
                    std::string material_name = "";
                    std::array<float, 4> base_colour = { 1.0f,0.0f,1.0f,1.0f };
                    float metalness = 0.0f;
                    float roughness = 0.8f;
                    std::array<float, 4> specular_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

                    if (material_idx != -1)
                    {
                        material_name = model->materials[material_idx].name;
                        //std::copy_n(model->materials[material_idx].pbrMetallicRoughness.baseColorFactor.begin(),4, base_colour.begin());
                        metalness = model->materials[material_idx].pbrMetallicRoughness.metallicFactor;
                        roughness = model->materials[material_idx].pbrMetallicRoughness.roughnessFactor;

                        auto c = model->materials[material_idx].pbrMetallicRoughness.baseColorFactor;
                        base_colour = {
                            static_cast<float>(c[0]) * (1.0f - metalness),
                            static_cast<float>(c[1]) * (1.0f - metalness),
                            static_cast<float>(c[2]) * (1.0f - metalness),
                            static_cast<float>(c[3])
                        };
                        // assume a specular color value of 0.04 (around plastic) as default value for dielectrics
                        specular_colour = {
                            (static_cast<float>(c[0]) * metalness) + 0.04f * (1.0f - metalness),
                            (static_cast<float>(c[1]) * metalness) + 0.04f * (1.0f - metalness),
                            (static_cast<float>(c[2]) * metalness) + 0.04f * (1.0f - metalness),
                            static_cast<float>(c[3])
                        };

                        if (model->materials[material_idx].pbrMetallicRoughness.baseColorTexture.index != -1)
                        {
                            //TODO base color texture
                        }

                        if (model->materials[material_idx].pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
                        {
                            //TODO metallic roughness texture
                            GenericTextureLayout layout;

                            auto& img = model->images[model->textures[model->materials[material_idx].pbrMetallicRoughness.metallicRoughnessTexture.index].source];

                            layout.width = img.width;
                            layout.height = img.height;
                            layout.depth = 1;
                            layout.type = img.pixel_type;
                            layout.format = 0x1908; // GL_RGBA, apparently tinygltf enforces 4 components for better vulkan compability anyway

                            auto APIlayout = m_rsrc_mngr.convertGenericTextureLayout(layout);

                            m_rsrc_mngr.createTexture2DAsync(
                                material_name + "_metallicRoughness",
                                APIlayout,
                                img.image.data());
                        }
                    }

                    std::string identifier_string =
                        "ga_" + model->nodes[gltf_node_idx].name + "_n_" + std::to_string(gltf_node_idx) + "_p_" + std::to_string(primitive_idx);

                    auto primitive_topology_type = m_rsrc_mngr.convertGenericPrimitiveTopology(0x0004/*GL_TRIANGLES*/);

                    EngineCore::Graphics::ResourceID mesh_rsrc = m_world.accessMeshComponentManager().addComponent(
                        entity,
                        identifier_string,
                        std::get<1>(mesh_data),
                        std::get<2>(mesh_data),
                        std::get<0>(mesh_data),
                        std::get<3>(mesh_data),
                        primitive_topology_type);

                    m_world.accessMaterialComponentManager().addComponent(entity, material_name, dflt_shader_prgm, base_colour, specular_colour, roughness);

                    size_t mesh_subidx = m_world.accessMeshComponentManager().getIndex(entity).size() - 1;
                    size_t mtl_subidx = m_world.accessMaterialComponentManager().getIndex(entity).size() - 1;;

                    //for (int subidx = 0; subidx < component_idxs.size(); ++subidx)
                    m_world.accessRenderTaskComponentManager().addComponent(entity, mesh_rsrc, mesh_subidx, dflt_shader_prgm, mtl_subidx);
                }
            }
        }

        template<typename ResourceManagerType>
        inline std::shared_ptr<tinygltf::Model> GltfAssetComponentManager<ResourceManagerType>::addGltfAsset(
            std::string const & gltf_filepath)
        {
            auto model = Utility::loadGltfModel(gltf_filepath);

            {
                std::unique_lock<std::shared_mutex> lock(m_gltf_assets_mutex);
                m_gltf_assets.insert(std::make_pair(gltf_filepath, model));
            }

            return model;
        }

        template<typename ResourceManagerType>
        inline void GltfAssetComponentManager<ResourceManagerType>::addGltfAsset(std::string const & gltf_filepath, ModelPtr const & gltf_model)
        {
            {
                std::unique_lock<std::shared_mutex> lock(m_gltf_assets_mutex);
                m_gltf_assets.insert(std::make_pair(gltf_filepath, gltf_model));
            }
        }

        template<typename ResourceManagerType>
        inline std::tuple<
            std::shared_ptr<typename ResourceManagerType::VertexLayout>,
            std::shared_ptr<std::vector<std::vector<unsigned char>>>,
            std::shared_ptr<std::vector<unsigned char>>,
            typename ResourceManagerType::IndexFormatType>
            GltfAssetComponentManager<ResourceManagerType>::loadMeshPrimitveData(ModelPtr const& model, size_t node_idx, size_t primitive_idx)
        {
            if (model == nullptr)
                return { nullptr,nullptr,nullptr, m_rsrc_mngr.convertGenericIndexType(0) };

            if (node_idx < model->nodes.size() && model->nodes[node_idx].mesh != -1)
            {
                auto& indices_accessor = model->accessors[model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].indices];
                auto& indices_bufferView = model->bufferViews[indices_accessor.bufferView];
                auto& indices_buffer = model->buffers[indices_bufferView.buffer];
                auto indices = std::make_shared<std::vector<unsigned char>>(
                    indices_buffer.data.begin() + indices_bufferView.byteOffset + indices_accessor.byteOffset,
                    indices_buffer.data.begin() + indices_bufferView.byteOffset + indices_accessor.byteOffset
                    + (indices_accessor.count * indices_accessor.ByteStride(indices_bufferView)));

                auto vertices = std::make_shared<std::vector<std::vector<unsigned char>>>();
                GenericVertexLayout generic_vertex_layout;
                generic_vertex_layout.byte_size = 0; //TODO rename byte_size

                unsigned int input_slot = 0;

                auto& vertex_attributes = model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].attributes;
                for (auto attrib : vertex_attributes)
                {
                    auto& vertexAttrib_accessor = model->accessors[attrib.second];
                    auto& vertexAttrib_bufferView = model->bufferViews[vertexAttrib_accessor.bufferView];
                    auto& vertexAttrib_buffer = model->buffers[vertexAttrib_bufferView.buffer];

                    generic_vertex_layout.attributes.push_back(
                        GenericVertexLayout::Attribute(attrib.first, vertexAttrib_accessor.type, vertexAttrib_accessor.componentType,
                            vertexAttrib_accessor.normalized, static_cast<uint32_t>(vertexAttrib_accessor.byteOffset)));

                    vertices->push_back(std::vector<unsigned char>(
                        vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset,
                        vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset
                        + (vertexAttrib_accessor.count * vertexAttrib_accessor.ByteStride(vertexAttrib_bufferView)))
                    );

                }

                auto vertex_layout = std::make_shared<typename ResourceManagerType::VertexLayout>(m_rsrc_mngr.convertGenericGltfVertexLayout(generic_vertex_layout));
                auto index_type = m_rsrc_mngr.convertGenericIndexType(indices_accessor.componentType);

                return { vertex_layout, vertices, indices, index_type };
            }
            else
            {
                return { nullptr,nullptr,nullptr,m_rsrc_mngr.convertGenericIndexType(0) };
            }
        }
    }
}

#endif // !GLTF_ASSET_COMPONENT_MANAGER
