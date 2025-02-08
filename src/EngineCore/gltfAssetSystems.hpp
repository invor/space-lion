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
            typedef std::shared_ptr<std::vector<GenericVertexLayout>>        VertexLayoutPtr;
            typedef std::shared_ptr<std::vector<std::vector<unsigned char>>> VertexDataPtr;
            typedef std::shared_ptr<std::vector<unsigned char>>              IndexDataPtr;
            //typedef typename ResourceManagerType::IndexFormatType            IndexDataType;
            typedef uint32_t                                                 IndexDataType;

            inline std::tuple<VertexLayoutPtr, VertexDataPtr, IndexDataPtr, IndexDataType>
                loadMeshPrimitveData(std::shared_ptr<tinygltf::Model> const& model, size_t node_idx, size_t primitive_idx)
            {
                if (model == nullptr)
                    return { nullptr,nullptr,nullptr, 0 };

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
                    std::shared_ptr<std::vector<GenericVertexLayout>> generic_vertex_layouts = std::make_shared<std::vector<GenericVertexLayout>>();

                    unsigned int input_slot = 0;

                    auto& vertex_attributes = model->meshes[model->nodes[node_idx].mesh].primitives[primitive_idx].attributes;
                    for (auto attrib : vertex_attributes)
                    {
                        auto& vertexAttrib_accessor = model->accessors[attrib.second];
                        auto& vertexAttrib_bufferView = model->bufferViews[vertexAttrib_accessor.bufferView];
                        auto& vertexAttrib_buffer = model->buffers[vertexAttrib_bufferView.buffer];

                        generic_vertex_layouts->push_back(GenericVertexLayout());

                        // Important note!: We ignore the byte offset given by gltf because we reorder vertex data in buffers anyway
                        generic_vertex_layouts->back().attributes.push_back(
                            GenericVertexLayout::Attribute(attrib.first, vertexAttrib_accessor.type, vertexAttrib_accessor.componentType,
                                vertexAttrib_accessor.normalized, 0 /*static_cast<uint32_t>(vertexAttrib_accessor.byteOffset)*/));

                        generic_vertex_layouts->back().stride = vertexAttrib_accessor.ByteStride(vertexAttrib_bufferView);

                        vertices->push_back(std::vector<unsigned char>(
                            vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset,
                            vertexAttrib_buffer.data.begin() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset
                            + (vertexAttrib_accessor.count * vertexAttrib_accessor.ByteStride(vertexAttrib_bufferView)))
                        );
                    }

                    auto index_type = static_cast<uint32_t>(indices_accessor.componentType);

                    return { generic_vertex_layouts, vertices, indices, index_type };
                }
                else
                {
                    return { nullptr,nullptr,nullptr,0 };
                }
            }

            inline std::tuple<std::shared_ptr<std::vector<unsigned char>>, std::shared_ptr<std::vector<uint32_t>>> loadMeshAttributeData(std::shared_ptr<tinygltf::Model> const& model, const std::string& attribute) {

                if (model != nullptr)
                {
                    auto attribues = std::make_shared<std::vector<unsigned char>>();
                    auto indices = std::make_shared<std::vector<uint32_t>>();

                    for (const auto& mesh : model->meshes) {
                        for (const auto& primitive : mesh.primitives)
                        {
                            const auto& indices_accessor = model->accessors[primitive.indices];
                            const auto& indices_bufferView = model->bufferViews[indices_accessor.bufferView];
                            const auto& indices_buffer = model->buffers[indices_bufferView.buffer];

                            if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                                const unsigned char* indices_data = reinterpret_cast<const unsigned char*>(&indices_buffer.data[indices_bufferView.byteOffset + indices_accessor.byteOffset]);
                                indices->assign(indices_data, indices_data + indices_accessor.count);
                            }
                            else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                                const unsigned int* indices_data = reinterpret_cast<const unsigned int*>(&indices_buffer.data[indices_bufferView.byteOffset + indices_accessor.byteOffset]);
                                indices->assign(indices_data, indices_data + indices_accessor.count);
                            }
                            else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                                const unsigned short* indices_data = reinterpret_cast<const unsigned short*>(&indices_buffer.data[indices_bufferView.byteOffset + indices_accessor.byteOffset]);
                                indices->assign(indices_data, indices_data + indices_accessor.count);
                            }

                            const auto& attribute_accessor = model->accessors[primitive.attributes.find(attribute)->second];
                            const auto& attribute_bufferView = model->bufferViews[attribute_accessor.bufferView];
                            const auto& attribute_buffer = model->buffers[attribute_bufferView.buffer];

                            const unsigned char* attribute_data = &attribute_buffer.data[attribute_bufferView.byteOffset + attribute_accessor.byteOffset];
                            attribues->insert(attribues->end(), attribute_data, attribute_data + (attribute_accessor.count * attribute_accessor.ByteStride(attribute_bufferView)));
                        }
                    }

                    return { attribues, indices };
                }
                else
                {
                    return { nullptr, nullptr };
                }
            }

            template<typename ResourceManagerType>
            inline void addGltfNode(
                EngineCore::WorldState& world_state,
                ResourceManagerType& resource_manager,
                std::string const& gltf_filepath,
                std::shared_ptr<tinygltf::Model> const& model,
                size_t gltf_node_idx,
                Entity parent_entity,
                std::unordered_map<int, Entity>& node_to_entity,
                ResourceID dflt_shader_prgm)
            {
                auto& gltf_asset_mngr = world_state.get<EngineCore::Graphics::GltfAssetComponentManager>();
                auto& name_mngr = world_state.get<EngineCore::Common::NameComponentManager>();
                auto& transform_mngr = world_state.get<EngineCore::Common::TransformComponentManager>();
                auto& mtl_mngr = world_state.get<EngineCore::Graphics::MaterialComponentManager>();
                auto& mesh_mngr = world_state.get<EngineCore::Graphics::MeshComponentManager<ResourceManagerType>>();
                auto& staticMesh_renderTask_mngr = world_state.get<EngineCore::Graphics::RenderTaskComponentManager<EngineCore::Graphics::RenderTaskTags::StaticMesh>>();
                auto& skinnedMesh_renderTask_mngr = world_state.get<EngineCore::Graphics::RenderTaskComponentManager<EngineCore::Graphics::RenderTaskTags::SkinnedMesh>>();
                auto& skin_mngr = world_state.get<EngineCore::Animation::SkinComponentManager>();

                // add entity
                auto entity = world_state.accessEntityManager().create();
                node_to_entity.insert({ gltf_node_idx,entity });

                gltf_asset_mngr.addComponent(entity, gltf_filepath, gltf_node_idx);

                // add name
                name_mngr.addComponent(entity, model->nodes[gltf_node_idx].name);

                // add transform
                auto transform_idx = transform_mngr.addComponent(entity);

                if (parent_entity != world_state.accessEntityManager().invalidEntity())
                {
                    transform_mngr.setParent(transform_idx, parent_entity);
                }

                // traverse children and add gltf nodes recursivly
                for (auto child : model->nodes[gltf_node_idx].children)
                {
                    addGltfNode(world_state, resource_manager, gltf_filepath, model, child, entity, node_to_entity, dflt_shader_prgm);
                }

                // add rendering components
                {
                    if (model->nodes[gltf_node_idx].matrix.size() != 0) // has matrix transform
                    {
                        glm::mat4 transformation = glm::make_mat4(model->nodes[gltf_node_idx].matrix.data()); // your transformation matrix.
                        glm::vec3 scale;
                        glm::quat rotation;
                        glm::vec3 translation;
                        glm::vec3 skew;
                        glm::vec4 perspective;
                        glm::decompose(transformation, scale, rotation, translation, skew, perspective);

                        transform_mngr.setPosition(transform_idx, translation);
                        transform_mngr.scale(transform_idx, scale);
                        transform_mngr.setOrientation(transform_idx, rotation);
                    }
                    else
                    {
                        auto& translation = model->nodes[gltf_node_idx].translation;
                        auto& scale = model->nodes[gltf_node_idx].scale;
                        auto& rotation = model->nodes[gltf_node_idx].rotation;

                        if (translation.size() != 0) {
                            transform_mngr.setPosition(
                                transform_idx,
                                Vec3(static_cast<float>(translation[0]),
                                    static_cast<float>(translation[1]),
                                    static_cast<float>(translation[2]))
                            );
                        }
                        if (scale.size() != 0) {
                            transform_mngr.scale(
                                transform_idx,
                                Vec3(static_cast<float>(scale[0]),
                                    static_cast<float>(scale[1]),
                                    static_cast<float>(scale[2]))
                            );
                        }
                        if (rotation.size() != 0) {
                            transform_mngr.setOrientation(
                                transform_idx,
                                Quat(static_cast<float>(rotation[3]),
                                    static_cast<float>(rotation[0]),
                                    static_cast<float>(rotation[1]),
                                    static_cast<float>(rotation[2]))
                            );
                        }
                    }

                    if (model->nodes[gltf_node_idx].skin != -1) {

                        auto skin = model->skins[model->nodes[gltf_node_idx].skin];

                        auto joint_cnt = skin.joints.size();

                        std::vector<Entity> joints;
                        joints.reserve(joint_cnt);
                        for (auto joint : skin.joints) {
                            // add entity
                            //      auto joint_entity = m_world.accessEntityManager().create();
                            //      node_to_entity.insert({joint,joint_entity});
                            // add transform
                            //      auto transform_idx = transform_mngr.addComponent(joint_entity);

                            auto joint_entity = node_to_entity.find(joint)->second;

                            joints.push_back(joint_entity);

                            {
                                //////
                                //
                                // TODO find a cross-API friendly way to do stuff like this
                                // start debugging skeleton joints
                                //
                                //////
                                //      auto primitive_topology_type = resource_manager.convertGenericPrimitiveTopology(0x0004/*GL_TRIANGLES*/);
                                //      const auto [vertex_data, index_data, vertex_description] = Graphics::createIcoSphere(2, 0.005);
                                //      auto gl_vertex_desc = std::make_shared<std::vector<ResourceManagerType::VertexLayout>>();
                                //      for (auto const& vertex_layout : (*vertex_description)) {
                                //          gl_vertex_desc->push_back(resource_manager.convertGenericGltfVertexLayout(vertex_layout));
                                //      }
                                //      EngineCore::Graphics::ResourceID mesh_rsrc = mesh_mngr.addComponent(
                                //          joint_entity,
                                //          "skeleton_bone",
                                //          vertex_data,
                                //          index_data,
                                //          gl_vertex_desc,
                                //          0x1405,
                                //          0x0004);
                                //      
                                //      auto bone_shader_names = std::make_shared<std::vector<EngineCore::Graphics::OpenGL::ResourceManager::ShaderFilename>>(
                                //          std::initializer_list<EngineCore::Graphics::OpenGL::ResourceManager::ShaderFilename>{
                                //              { "../space-lion/resources/shaders/debug/skinned_mesh_bone_v.glsl", glowl::GLSLProgram::ShaderType::Vertex },
                                //              { "../space-lion/resources/shaders/debug/skinned_mesh_bone_f.glsl", glowl::GLSLProgram::ShaderType::Fragment }
                                //      });
                                //      auto bone_shader_rsrc = resource_manager.createShaderProgramAsync(
                                //          "skeleton_bone_shader",
                                //          bone_shader_names
                                //      );
                                //      using TextureSemantic = typename Graphics::MaterialComponentManager::TextureSemantic;
                                //      mtl_mngr.addComponent(
                                //          joint_entity,
                                //          "skeleton_bone",
                                //          dflt_shader_prgm,
                                //          { 1.0f,0.0f,1.0f,1.0f },
                                //          { 1.0f, 1.0f, 1.0f, 1.0f },
                                //          0.8f,
                                //          std::vector<std::pair<TextureSemantic, ResourceID>>{}
                                //      );
                                //      
                                //      staticMesh_renderTask_mngr.addComponent(
                                //          joint_entity,
                                //          mesh_rsrc,
                                //          0,
                                //          bone_shader_rsrc,
                                //          0,
                                //          transform_mngr.getIndex(joint_entity),
                                //          mesh_mngr.getIndex(joint_entity)[0],
                                //          mtl_mngr.getIndex(joint_entity)[0]
                                //      );
                                //////
                                // end debugging skeleton joints
                                //////
                            }
                        }

                        // build hierachy in transform manager after all joint nodes are added
                        //  for (auto joint : skin.joints) {
                        //      auto parent_query = node_to_entity.find(joint);
                        //  
                        //      if (parent_query != node_to_entity.end()) {
                        //          for (auto child : model->nodes[joint].children) {
                        //              auto child_query = node_to_entity.find(child);
                        //  
                        //              if (child_query != node_to_entity.end()) {
                        //                  transform_mngr.setParent(transform_mngr.getIndex(child_query->second.id()), parent_query->second);
                        //              }
                        //          }
                        //      }
                        //  }


                        std::vector<Mat4x4> inverse_bind_matrices(joint_cnt);
                        auto const& matrices_accessor = model->accessors[skin.inverseBindMatrices];
                        auto const& matrices_bufferView = model->bufferViews[matrices_accessor.bufferView];
                        auto const& matrices_buffer = model->buffers[matrices_bufferView.buffer];

                        assert((inverse_bind_matrices.size() * sizeof(Mat4x4)) == matrices_bufferView.byteLength);

                        std::copy(
                            matrices_buffer.data.data() + matrices_bufferView.byteOffset,
                            matrices_buffer.data.data() + matrices_bufferView.byteOffset + matrices_bufferView.byteLength,
                            reinterpret_cast<uint8_t*>(inverse_bind_matrices.data()));

                        skin_mngr.addComponent(entity, joints, inverse_bind_matrices);
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
                            //    (max + min) * 0.5f,
                            //    (max - min) * 0.5f));

                            auto mesh_data = loadMeshPrimitveData(model, gltf_node_idx, primitive_idx);

                            // check mesh data for tangents, compute tangents if not available but normals+uvs are given
                            bool has_tangents = false;
                            bool has_normals = false;
                            bool has_uvs = false;
                            int position_data_idx = -1;
                            int normal_data_idx = -1;
                            int uv_data_idx = -1;
                            int tangent_data_idx = -1;

                            int curr_idx = 0;
                            for (auto& generic_vertex_layout : (*(std::get<0>(mesh_data))))
                            {
                                for (auto& attrib : generic_vertex_layout.attributes) {
                                    // note: because gltf data is reordered during loading, we know here that there is only one attribute per layout
                                    if (attrib.semantic_name == "POSITION")
                                    {
                                        position_data_idx = curr_idx;
                                    }
                                    else if (attrib.semantic_name == "NORMAL")
                                    {
                                        has_normals = true;
                                        normal_data_idx = curr_idx;
                                    }
                                    else if (attrib.semantic_name == "TEXCOORD_0")
                                    {
                                        has_uvs = true;
                                        uv_data_idx = curr_idx;
                                    }
                                    else if (attrib.semantic_name == "TANGENT")
                                    {
                                        has_tangents = true;
                                        tangent_data_idx = curr_idx;
                                    }
                                }
                                ++curr_idx;
                            }

                            if ((!has_tangents) && has_normals && has_uvs)
                            {

                                // add data buffer and layout entry for tangents
                                {
                                    auto it = std::get<0>(mesh_data)->insert(std::get<0>(mesh_data)->begin() + 2, GenericVertexLayout());
                                    it->stride = 4 * 4;
                                    it->attributes.push_back(
                                        GenericVertexLayout::Attribute("TANGENT", 4, 5126, false, 0 /*static_cast<uint32_t>(vertexAttrib_accessor.byteOffset)*/));
                                }

                                {
                                    auto it = std::get<1>(mesh_data)->insert(std::get<1>(mesh_data)->begin() + 2, std::vector<unsigned char>());

                                    //TODO handle insertion of vertex layout better, i.e. decouple shader ordering from layout ordering...
                                    curr_idx = 0;
                                    for (auto& generic_vertex_layout : (*(std::get<0>(mesh_data))))
                                    {
                                        for (auto& attrib : generic_vertex_layout.attributes) {
                                            // note: because gltf data is reordered during loading, we know here that there is only one attribute per layout
                                            if (attrib.semantic_name == "POSITION")
                                            {
                                                position_data_idx = curr_idx;
                                            }
                                            else if (attrib.semantic_name == "NORMAL")
                                            {
                                                normal_data_idx = curr_idx;
                                            }
                                            else if (attrib.semantic_name == "TEXCOORD_0")
                                            {
                                                uv_data_idx = curr_idx;
                                            }
                                            else if (attrib.semantic_name == "TANGENT")
                                            {
                                                tangent_data_idx = curr_idx;
                                            }
                                        }
                                        ++curr_idx;
                                    }


                                    it->resize(((*std::get<1>(mesh_data))[position_data_idx].size() / 3) * 4); //TODO: proper getByteSize for generic layout
                                }

                                if (std::get<3>(mesh_data) == 5123) // check whether index type is 16bit (uses GL enums as generic types)
                                {
                                    makeTangents(
                                        std::get<2>(mesh_data)->size() / 2,
                                        reinterpret_cast<uint16_t*>(std::get<2>(mesh_data)->data()),
                                        reinterpret_cast<glm::vec3*>((*std::get<1>(mesh_data))[position_data_idx].data()),
                                        reinterpret_cast<glm::vec3*>((*std::get<1>(mesh_data))[normal_data_idx].data()),
                                        reinterpret_cast<glm::vec2*>((*std::get<1>(mesh_data))[uv_data_idx].data()),
                                        reinterpret_cast<glm::vec4*>((*std::get<1>(mesh_data))[tangent_data_idx].data())
                                    );
                                }
                                else if (std::get<3>(mesh_data) == 5125) // check whether index type is 32bit (uses GL enums as generic types)
                                {
                                    makeTangents(
                                        std::get<2>(mesh_data)->size() / 4,
                                        reinterpret_cast<uint32_t*>(std::get<2>(mesh_data)->data()),
                                        reinterpret_cast<glm::vec3*>((*std::get<1>(mesh_data))[position_data_idx].data()),
                                        reinterpret_cast<glm::vec3*>((*std::get<1>(mesh_data))[normal_data_idx].data()),
                                        reinterpret_cast<glm::vec2*>((*std::get<1>(mesh_data))[uv_data_idx].data()),
                                        reinterpret_cast<glm::vec4*>((*std::get<1>(mesh_data))[tangent_data_idx].data())
                                    );
                                }
                            }

                            auto material_idx = model->meshes[model->nodes[gltf_node_idx].mesh].primitives[primitive_idx].material;
                            std::string material_name = "";
                            std::array<float, 4> base_colour = { 1.0f,0.0f,1.0f,1.0f };
                            float metalness = 0.0f;
                            float roughness = 0.8f;
                            std::array<float, 4> specular_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

                            typedef MaterialComponentManager::TextureSemantic TextureSemantic;
                            std::vector< std::pair<TextureSemantic, ResourceID>> textures;

                            std::string identifier_string =
                                "ga_" + model->nodes[gltf_node_idx].name + "_n_" + std::to_string(gltf_node_idx) + "_p_" + std::to_string(primitive_idx);

                            if (material_idx != -1)
                            {
                                material_name = model->materials[material_idx].name.empty() ? identifier_string : model->materials[material_idx].name;
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
                                    // base color texture (diffuse albedo)
                                    GenericTextureLayout layout;

                                    auto& img = model->images[model->textures[model->materials[material_idx].pbrMetallicRoughness.baseColorTexture.index].source];

                                    layout.width = img.width;
                                    layout.height = img.height;
                                    layout.depth = 1;
                                    layout.levels = 1;
                                    layout.type = img.pixel_type;
                                    layout.format = 0x1908; // GL_RGBA, apparently tinygltf enforces 4 components for better vulkan compability anyway
                                    layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;// GL_RGBA8

                                    layout.int_parameters = {
                                        {
                                            0x2801,// GL_TEXTURE_MIN_FILTER
                                            0x2703 //GL_LINEAR_MIPMAP_LINEAR
                                        },
                                        {
                                            0x2800, //GL_TEXTURE_MAG_FILTER
                                            0x2601 //GL_LINEAR
                                        }
                                    };
                                    layout.float_parameters = {
                                        {
                                            0x84FE, //GL_TEXTURE_MAX_ANISOTROPY_EXT
                                            8.0f
                                        }
                                    };

                                    auto APIlayout = resource_manager.convertGenericTextureLayout(layout);

                                    auto tx_rsrcID = resource_manager.createTexture2DAsync(
                                        material_name + "_baseColor",
                                        APIlayout,
                                        img.image.data(),
                                        true);

                                    textures.emplace_back(std::make_pair(TextureSemantic::ALBEDO, tx_rsrcID));
                                }

                                if (model->materials[material_idx].pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
                                {
                                    // metallic roughness texture
                                    GenericTextureLayout layout;

                                    auto& img = model->images[model->textures[model->materials[material_idx].pbrMetallicRoughness.metallicRoughnessTexture.index].source];

                                    layout.width = img.width;
                                    layout.height = img.height;
                                    layout.depth = 1;
                                    layout.levels = 1;
                                    layout.type = img.pixel_type;
                                    layout.format = 0x1908; // GL_RGBA, apparently tinygltf enforces 4 components for better vulkan compability anyway
                                    layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;// GL_RGBA8

                                    layout.int_parameters = {
                                        {
                                            0x2801,// GL_TEXTURE_MIN_FILTER
                                            0x2703 //GL_LINEAR_MIPMAP_LINEAR
                                        },
                                        {
                                            0x2800, //GL_TEXTURE_MAG_FILTER
                                            0x2601 //GL_LINEAR
                                        }
                                    };
                                    layout.float_parameters = {
                                        {
                                            0x84FE, //GL_TEXTURE_MAX_ANISOTROPY_EXT
                                            8.0f
                                        }
                                    };

                                    auto APIlayout = resource_manager.convertGenericTextureLayout(layout);

                                    auto tx_rsrcID = resource_manager.createTexture2DAsync(
                                        material_name + "_metallicRoughness",
                                        APIlayout,
                                        img.image.data(),
                                        true);

                                    textures.emplace_back(std::make_pair(TextureSemantic::METALLIC_ROUGHNESS, tx_rsrcID));
                                }

                                if (model->materials[material_idx].normalTexture.index != -1)
                                {
                                    // normal map texture
                                    GenericTextureLayout layout;

                                    auto& img = model->images[model->textures[model->materials[material_idx].normalTexture.index].source];

                                    layout.width = img.width;
                                    layout.height = img.height;
                                    layout.depth = 1;
                                    layout.levels = 1;
                                    layout.type = img.pixel_type;
                                    layout.format = 0x1908; // GL_RGBA, apparently tinygltf enforces 4 components for better vulkan compability anyway
                                    layout.internal_format = GenericTextureLayout::InternalFormat::RGBA8;// GL_RGBA8

                                    layout.int_parameters = {
                                        {
                                            0x2801,// GL_TEXTURE_MIN_FILTER
                                            0x2703 //GL_LINEAR_MIPMAP_LINEAR
                                        },
                                        {
                                            0x2800, //GL_TEXTURE_MAG_FILTER
                                            0x2601 //GL_LINEAR
                                        }
                                    };
                                    layout.float_parameters = {
                                        {
                                            0x84FE, //GL_TEXTURE_MAX_ANISOTROPY_EXT
                                            8.0f
                                        }
                                    };

                                    auto APIlayout = resource_manager.convertGenericTextureLayout(layout);

                                    auto tx_rsrcID = resource_manager.createTexture2DAsync(
                                        material_name + "_normal",
                                        APIlayout,
                                        img.image.data(),
                                        true);

                                    textures.emplace_back(std::make_pair(TextureSemantic::NORMAL, tx_rsrcID));
                                }
                            }

                            auto primitive_topology_type = resource_manager.convertGenericPrimitiveTopology(0x0004/*GL_TRIANGLES*/);

                            EngineCore::Graphics::ResourceID mesh_rsrc = mesh_mngr.addComponent(
                                entity,
                                identifier_string,
                                std::get<1>(mesh_data),
                                std::get<2>(mesh_data),
                                std::get<0>(mesh_data),
                                std::get<3>(mesh_data),
                                primitive_topology_type);

                            mtl_mngr.addComponent(entity, material_name, dflt_shader_prgm, base_colour, specular_colour, roughness, textures);

                            size_t mesh_subidx = mesh_mngr.getIndex(entity).size() - 1;
                            size_t mtl_subidx = mtl_mngr.getIndex(entity).size() - 1;;

                            //for (int subidx = 0; subidx < component_idxs.size(); ++subidx)
                            if (model->nodes[gltf_node_idx].skin != -1) {
                                skinnedMesh_renderTask_mngr.addComponent(
                                    entity,
                                    mesh_rsrc,
                                    mesh_subidx,
                                    dflt_shader_prgm,
                                    mtl_subidx,
                                    transform_mngr.getIndex(entity),
                                    mesh_mngr.getIndex(entity)[mesh_subidx],
                                    mtl_mngr.getIndex(entity)[mtl_subidx]
                                );
                            }
                            else {
                                staticMesh_renderTask_mngr.addComponent(
                                    entity,
                                    mesh_rsrc,
                                    mesh_subidx,
                                    dflt_shader_prgm,
                                    mtl_subidx,
                                    transform_mngr.getIndex(entity),
                                    mesh_mngr.getIndex(entity)[mesh_subidx],
                                    mtl_mngr.getIndex(entity)[mtl_subidx]
                                );
                            }

                            // TODO experiment with index caching for better performance...
                            //renderTask_mngr.getComponentData().back().cached_transform_idx = transform_mngr.getIndex(entity).front();
                            //renderTask_mngr.getComponentData().back().cached_mesh_idx = mesh_mngr.getIndex(entity)[mesh_subidx];
                            //renderTask_mngr.getComponentData().back().cached_material_idx = mtl_mngr.getIndex(entity)[mtl_subidx];
                        }
                    }
                }
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

            auto gltf_model = gltf_asset_mngr.addGltfModelToCache(gltf_filepath);

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
                        gltf_filepath,
                        gltf_model,
                        node,
                        world_state.accessEntityManager().invalidEntity(),
                        node_to_entity,
                        dflt_shader_prgm
                    );
                }
            }

            std::vector<Entity> retval;

            for (auto& v : node_to_entity) {
                retval.push_back(v.second);
            }

            return retval;
        }

        std::vector<Entity> importGltfNode();
    }
}

#endif // !gltfAssetSystems_hpp
