#include "BasicRenderingPipeline.hpp"

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>

#if  1

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {
            void setupBasicForwardRenderingPipeline(
                Common::Frame & frame,
                WorldState & world_state,
                ResourceManager & resource_mngr)
            {
                struct GeomPassData
                {
#pragma pack(push, 1)
                    struct DrawElementsCommand
                    {
                        GLuint cnt;
                        GLuint instance_cnt;
                        GLuint first_idx;
                        GLuint base_vertex;
                        GLuint base_instance;
                    };
#pragma pack(pop)

                    struct StaticMeshParams
                    {
                        Mat4x4 transform;

                        GLuint64 base_color_tx_hndl;
                        GLuint64 roughnes_tx_hndl;
                        GLuint64 normal_tx_hndl;

                        GLuint64 padding;
                    };

                    // static mesh (shader) params per object per batch
                    std::vector<std::vector<StaticMeshParams>>    static_mesh_params;
                    std::vector<std::vector<DrawElementsCommand>> static_mesh_drawCommands;

                    std::vector<std::vector<WeakResource<glowl::Texture2D>>> tx_rsrc_access_cache;

                    Mat4x4 view_matrix;
                    Mat4x4 proj_matrix;
                };

                struct GeomPassResources
                {
                    struct BatchResources
                    {
                        WeakResource<glowl::GLSLProgram>  shader_prgm;
                        WeakResource<glowl::BufferObject> object_params;
                        WeakResource<glowl::BufferObject> draw_commands;
                        WeakResource<glowl::Mesh>         geometry;
                    };

                    std::vector<BatchResources> m_batch_resources;
                };

                frame.addRenderPass<GeomPassData, GeomPassResources>("GeometryPass",
                    // data setup phase
                    [&frame, &world_state, &resource_mngr](GeomPassData& data, GeomPassResources& resources) {

                    auto& cam_mngr = world_state.get<CameraComponentManager>();
                    auto& mtl_mngr = world_state.get<MaterialComponentManager<ResourceManager>>();
                    auto& mesh_mngr = world_state.get<MeshComponentManager<ResourceManager>> ();
                    auto& renderTask_mngr = world_state.get<RenderTaskComponentManager>();
                    auto& transform_mngr = world_state.get<Common::TransformComponentManager>();

                    // set camera matrices
                    Entity camera_entity = cam_mngr.getActiveCamera();
                    auto camera_idx = cam_mngr.getIndex(camera_entity).front();
                    auto camera_transform_idx = transform_mngr.getIndex(camera_entity);
                    if (!camera_transform_idx.empty())
                    {
                        data.view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx.front()));
                    }
                    cam_mngr.setAspectRatio(camera_idx, static_cast<float>(frame.m_window_width) / static_cast<float>(frame.m_window_height) );
                    cam_mngr.updateProjectionMatrix(camera_idx);
                    data.proj_matrix = cam_mngr.getProjectionMatrix(camera_idx);

                    // set per object data
                    auto objs = renderTask_mngr.getComponentDataCopy();

                    ResourceID current_prgm = resource_mngr.invalidResourceID();
                    ResourceID current_mesh = resource_mngr.invalidResourceID();

                    // iterate all objects
                    for (auto& obj : objs)
                    {
                        // create a new batch for each resource change
                        if (obj.shader_prgm != current_prgm || obj.mesh != current_mesh)
                        {
                            current_prgm = obj.shader_prgm;
                            current_mesh = obj.mesh;

                            auto batch_idx = data.static_mesh_params.size();
                            data.static_mesh_params.push_back(std::vector<GeomPassData::StaticMeshParams>());
                            data.static_mesh_drawCommands.push_back(std::vector<GeomPassData::DrawElementsCommand>());

                            data.tx_rsrc_access_cache.push_back(std::vector<WeakResource<glowl::Texture2D>>());

                            // TODO query batch GPU resources early?
                            GeomPassResources::BatchResources batch_resources;
                            batch_resources.shader_prgm = resource_mngr.getShaderProgramResource(current_prgm);
                            batch_resources.object_params = resource_mngr.getBufferResource( "geomPass_obj_params_" + std::to_string(batch_idx) + "_" + std::to_string( frame.m_frameID % 2) );
                            batch_resources.draw_commands = resource_mngr.getBufferResource( "geomPass_draw_commands_" + std::to_string(batch_idx) + "_" + std::to_string(frame.m_frameID % 2) );
                            batch_resources.geometry = resource_mngr.getMeshResource(current_mesh);
                            resources.m_batch_resources.push_back(batch_resources);
                        }

                        // gather all per object information
                        GeomPassData::StaticMeshParams params;
                        auto transform_idx = transform_mngr.getIndex(obj.entity);
                        if (!transform_idx.empty()){
                            params.transform = transform_mngr.getWorldTransformation(transform_idx.front());
                        }
                        
                        auto mtl_idx = mtl_mngr.getIndex(obj.entity);
                        if (!mtl_idx.empty())
                        {
                            using TextureSemantic = MaterialComponentManager<ResourceManager>::TextureSemantic;
                            auto albedo_textures = mtl_mngr.getTextures(mtl_idx[obj.mtl_component_subidx], TextureSemantic::ALBEDO);
                            auto roughness_textures = mtl_mngr.getTextures(mtl_idx[obj.mtl_component_subidx], TextureSemantic::METALLIC_ROUGHNESS);
                            auto normal_textures = mtl_mngr.getTextures(mtl_idx[obj.mtl_component_subidx], TextureSemantic::NORMAL);

                            if (!albedo_textures.empty())
                            {
                                auto albedo_tx = resource_mngr.getTexture2DResource(albedo_textures[0]);
                                
                                if (albedo_tx.state == READY)
                                {
                                    params.base_color_tx_hndl = albedo_tx.resource->getTextureHandle();
                                    data.tx_rsrc_access_cache.back().push_back(albedo_tx);
                                }
                            }

                            if (!roughness_textures.empty())
                            {
                                auto roughness_tx = resource_mngr.getTexture2DResource(roughness_textures[0]);

                                if (roughness_tx.state == READY)
                                {
                                    params.roughnes_tx_hndl = roughness_tx.resource->getTextureHandle();
                                    data.tx_rsrc_access_cache.back().push_back(roughness_tx);
                                }
                            }

                            if (!normal_textures.empty())
                            {
                                auto normal_tx = resource_mngr.getTexture2DResource(normal_textures[0]);

                                if (normal_tx.state == READY)
                                {
                                    params.normal_tx_hndl = normal_tx.resource->getTextureHandle();
                                    data.tx_rsrc_access_cache.back().push_back(normal_tx);
                                }
                            }
                        }

                        data.static_mesh_params.back().push_back(params);

                        // set draw command values
                        GeomPassData::DrawElementsCommand draw_command;
                        auto obj_mesh_idx = mesh_mngr.getIndex(obj.entity);
                        auto draw_params = mesh_mngr.getDrawIndexedParams(obj_mesh_idx[obj.mesh_component_subidx]);
                        draw_command.cnt = std::get<0>(draw_params);
                        draw_command.base_vertex = std::get<2>(draw_params);
                        draw_command.first_idx = std::get<1>(draw_params);
                        draw_command.base_instance = 0;
                        draw_command.instance_cnt = 1;
                        data.static_mesh_drawCommands.back().push_back(draw_command);
                    }
                },
                    // resource setup phase
                    [&frame, &world_state, &resource_mngr](GeomPassData& data, GeomPassResources& resources) {

                    // buffer data to resources
                    uint batch = 0;
                    for (auto& batch_resources : resources.m_batch_resources)
                    {
                        if (batch_resources.shader_prgm.state != READY)
                        {
                            //???
                        }

                        if (batch_resources.geometry.state != READY)
                        {
                            //???
                        }

                        {
                            // Try to guarantee texture residency
                            for (auto& tx_rsrc : data.tx_rsrc_access_cache[batch]) {                                
                                if(!glIsTextureHandleResidentARB(tx_rsrc.resource->getTextureHandle())){
                                    tx_rsrc.resource->makeResident();
                                }
                            }
                        }

                        if (batch_resources.object_params.state != READY)
                        {
                            batch_resources.object_params = resource_mngr.createBufferObject(
                                "geomPass_obj_params_" + std::to_string(batch) + "_" + std::to_string(frame.m_frameID % 2),
                                GL_SHADER_STORAGE_BUFFER,
                                data.static_mesh_params[batch]
                            );
                        }
                        else
                        {
                            batch_resources.object_params.resource->rebuffer(data.static_mesh_params[batch]);
                        }

                        if (batch_resources.draw_commands.state != READY)
                        {
                            batch_resources.draw_commands = resource_mngr.createBufferObject(
                                "geomPass_draw_commands_" + std::to_string(batch) + "_" + std::to_string(frame.m_frameID % 2),
                                GL_DRAW_INDIRECT_BUFFER,
                                data.static_mesh_drawCommands[batch]
                            );
                        }
                        else
                        {
                            batch_resources.draw_commands.resource->rebuffer(data.static_mesh_drawCommands[batch]);
                        }

                        ++batch;

                        auto gl_err = glGetError();
                        if (gl_err != GL_NO_ERROR)
                            std::cerr << "GL error in geometry pass resource setup - batch " << batch << " : " << gl_err << std::endl;
                    }
                },
                    // execute phase
                    [&frame](GeomPassData const& data, GeomPassResources const& resources) {

                    glEnable(GL_CULL_FACE);
                    glFrontFace(GL_CCW);
                    glEnable(GL_DEPTH_TEST);

                    //glDisable(GL_BLEND);
                    //glDisable(GL_CULL_FACE);

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glClearColor(0.2f, 0.2f, 0.2f, 1);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glViewport(0, 0, frame.m_window_width, frame.m_window_height);

                    // bind global resources?

                    uint batch_idx = 0;
                    for (auto& batch_resources : resources.m_batch_resources)
                    {
                        if (batch_resources.shader_prgm.state != READY || batch_resources.geometry.state != READY)
                            continue;

                        batch_resources.shader_prgm.resource->use();

                        batch_resources.shader_prgm.resource->setUniform("view_matrix", data.view_matrix);
                        batch_resources.shader_prgm.resource->setUniform("projection_matrix", data.proj_matrix);

                        batch_resources.object_params.resource->bind(0);

                        batch_resources.draw_commands.resource->bind();
                        batch_resources.geometry.resource->bindVertexArray();

                        GLsizei draw_cnt = static_cast<GLsizei>( data.static_mesh_drawCommands[batch_idx].size() );

                        auto gl_err = glGetError();
                        if (gl_err != GL_NO_ERROR)
                            std::cerr << "GL error in geometry pass execution - batch "<< batch_idx<<" : " << gl_err << std::endl;

                        glMultiDrawElementsIndirect(
                            batch_resources.geometry.resource->getPrimitiveType(),
                            batch_resources.geometry.resource->getIndexType(),
                            (GLvoid*)0,
                            draw_cnt,
                            0);
                        //glDrawArrays(GL_TRIANGLES, 0, 6);

                        ++batch_idx;
                    }

                    //glDisable(GL_CULL_FACE);

                    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    auto gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR) {
                        std::cerr << "GL error in geometry pass : " << gl_err << std::endl;
                    }


                    ImGui::SetNextWindowPos(ImVec2(frame.m_window_width - 375.0f, frame.m_window_height - 135.0f));
                    bool p_open = true;
                    if (!ImGui::Begin("Render Stats", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
                    {
                        ImGui::End();
                        return;
                    }
                    ImGui::Text("# batches (draw calls): %u ", batch_idx);
                    ImGui::End();
                }
                );
            }
        }
    }
}
#endif

#if  0

namespace EngineCore
{
    namespace Graphics
    {
        namespace OpenGL
        {

            void setupBasicRenderingPipeline(
                Common::Frame& frame,
                WorldState& world_state,
                ResourceManager& resource_mngr)
            {
                struct InitPassData
                {
                };

                struct InitPassResources
                {
                    WeakResource<FramebufferObject> m_gBuffer;
                };

                frame.addRenderPass<InitPassData, InitPassResources>("InitPass",
                    [&world_state, &resource_mngr](InitPassData& data, InitPassResources& resources) {
                    // check for existing gBuffer
                    resources.m_gBuffer = resource_mngr.getFramebufferObject("gBuffer");
                },
                    [&world_state, &resource_mngr](InitPassData& data, InitPassResources& resources) {
                    if (resources.m_gBuffer.state != READY)
                    {
                        resources.m_gBuffer = resource_mngr.createFramebufferObject("gBuffer", 1600, 900);
                        resources.m_gBuffer.resource->createColorAttachment(GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV);
                        resources.m_gBuffer.resource->createColorAttachment(GL_R32F, GL_RED, GL_FLOAT);
                        resources.m_gBuffer.resource->createColorAttachment(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
                        resources.m_gBuffer.resource->createColorAttachment(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
                        resources.m_gBuffer.resource->createColorAttachment(GL_R32I, GL_RED_INTEGER, GL_INT);
                    }
                },
                    [](InitPassData const& data, InitPassResources const& resources) {}
                );


                struct GeomPassData
                {
                    struct DrawElementsCommand
                    {
                        GLuint cnt;
                        GLuint instance_cnt;
                        GLuint first_idx;
                        GLuint base_vertex;
                        GLuint base_instance;
                    };

                    struct StaticMeshParams
                    {
                        Mat4x4 transform;
                    };

                    // static mesh (shader) params per object per batch
                    std::vector<std::vector<StaticMeshParams>>		static_mesh_params;
                    std::vector<std::vector<DrawElementsCommand>>	static_mesh_drawCommands;

                    Mat4x4 view_matrix;
                    Mat4x4 proj_matrix;
                };

                struct GeomPassResources
                {
                    struct BatchResources
                    {
                        WeakResource<GLSLProgram>	shader_prgm;
                        WeakResource<BufferObject>	object_params;
                        WeakResource<BufferObject>	draw_commands;
                        WeakResource<Mesh>			geometry;
                    };

                    std::vector<BatchResources> m_batch_resources;

                    WeakResource<FramebufferObject> m_render_target;
                };

                frame.addRenderPass<GeomPassData, GeomPassResources>("GeometryPass",
                    // data setup phase
                    [&world_state, &resource_mngr](GeomPassData& data, GeomPassResources& resources) {

                    // try to get render target early
                    resources.m_render_target = resource_mngr.getFramebufferObject("gBuffer");

                    // set camera matrices
                    uint camera_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
                    Entity camera_entity = GCoreComponents::cameraManager().getEntity(camera_idx);
                    uint camera_transform_idx = GCoreComponents::transformManager().getIndex(camera_entity);
                    data.view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(camera_transform_idx));
                    data.proj_matrix = GCoreComponents::cameraManager().getProjectionMatrix(camera_idx);

                    // set per object data
                    auto& objs = GRenderingComponents::simpleObjectDrawManager().getDrawObjects();

                    ResourceID current_mtl = resource_mngr.getInvalidResourceID();
                    ResourceID current_mesh = resource_mngr.getInvalidResourceID();

                    // iterate all objects
                    for (auto& obj : objs)
                    {
                        // create a new batch for each resource change
                        if (obj.material_resource != current_mtl || obj.mesh_resource != current_mesh)
                        {
                            current_mtl = obj.material_resource;
                            current_mesh = obj.mesh_resource;

                            data.static_mesh_params.push_back(std::vector<GeomPassData::StaticMeshParams>());
                            data.static_mesh_drawCommands.push_back(std::vector<GeomPassData::DrawElementsCommand>());

                            // TODO query batch GPU resources early
                            GeomPassResources::BatchResources batch_resources;
                            batch_resources.shader_prgm = resource_mngr.getGLSLProgram(current_mtl);
                            batch_resources.object_params = resource_mngr.getBufferObject("geometryPass_object_parameters");
                            batch_resources.draw_commands = resource_mngr.getBufferObject("geometryPass_draw_commands");
                            batch_resources.geometry = resource_mngr.getMesh(current_mesh);
                            resources.m_batch_resources.push_back(batch_resources);
                        }


                        GeomPassData::StaticMeshParams params;
                        uint transform_idx = GCoreComponents::transformManager().getIndex(obj.entity);
                        params.transform = GCoreComponents::transformManager().getWorldTransformation(transform_idx);
                        data.static_mesh_params.back().push_back(params);

                        // set draw command values
                        GeomPassData::DrawElementsCommand draw_command;
                        auto obj_mesh_idx = GRenderingComponents::meshManager().getIndex(obj.entity);
                        draw_command.cnt = GRenderingComponents::meshManager().getComponents().at(std::get<1>(obj_mesh_idx)).indices_cnt;
                        draw_command.base_vertex = GRenderingComponents::meshManager().getComponents().at(std::get<1>(obj_mesh_idx)).base_vertex;
                        draw_command.first_idx = GRenderingComponents::meshManager().getComponents().at(std::get<1>(obj_mesh_idx)).first_index;
                        draw_command.base_instance = 0;
                        draw_command.instance_cnt = 1;
                        data.static_mesh_drawCommands.back().push_back(draw_command);
                    }
                },
                    // resource setup phase
                    [&world_state, &resource_mngr](GeomPassData& data, GeomPassResources& resources) {

                    if (resources.m_render_target.state == NOT_READY)
                        resources.m_render_target = resource_mngr.getFramebufferObject("gBuffer");

                    // buffer data to resources
                    uint batch = 0;
                    for (auto& batch_resources : resources.m_batch_resources)
                    {
                        if (batch_resources.shader_prgm.state != READY)
                        {
                            //???
                        }

                        if (batch_resources.geometry.state != READY)
                        {
                            //???
                        }

                        if (batch_resources.object_params.state != READY)
                        {
                            batch_resources.object_params = resource_mngr.createBufferObject("geometryPass_object_parameters", GL_SHADER_STORAGE_BUFFER, data.static_mesh_params[batch]);
                        }
                        else
                        {
                            batch_resources.object_params.resource->rebuffer(data.static_mesh_params[batch]);
                        }

                        if (batch_resources.draw_commands.state != READY)
                        {
                            batch_resources.draw_commands = resource_mngr.createBufferObject("geometryPass_draw_commands", GL_DRAW_INDIRECT_BUFFER, data.static_mesh_drawCommands[batch]);
                        }
                        else
                        {
                            batch_resources.draw_commands.resource->rebuffer(data.static_mesh_drawCommands[batch]);
                        }

                        ++batch;
                    }
                },
                    // execute phase
                    [](GeomPassData const& data, GeomPassResources const& resources) {

                    if (resources.m_render_target.state != READY)
                        return;

                    //glEnable(GL_CULL_FACE);
                    //glFrontFace(GL_CCW);

                    glDisable(GL_BLEND);
                    // bind render target and set viewport
                    resources.m_render_target.resource->bind();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    int width = resources.m_render_target.resource->getWidth();
                    int height = resources.m_render_target.resource->getHeight();
                    glViewport(0, 0, width, height);

                    // bind global resources?

                    uint batch_idx = 0;
                    for (auto& batch_resources : resources.m_batch_resources)
                    {
                        if (batch_resources.shader_prgm.state != READY || batch_resources.geometry.state != READY)
                            continue;

                        batch_resources.shader_prgm.resource->use();

                        batch_resources.shader_prgm.resource->setUniform("view_matrix", data.view_matrix);
                        batch_resources.shader_prgm.resource->setUniform("projection_matrix", data.proj_matrix);

                        batch_resources.object_params.resource->bind(0);

                        batch_resources.draw_commands.resource->bind();
                        batch_resources.geometry.resource->bindVertexArray();

                        GLsizei draw_cnt = data.static_mesh_drawCommands[batch_idx].size();

                        glMultiDrawElementsIndirect(
                            batch_resources.geometry.resource->getPrimitiveType(),
                            batch_resources.geometry.resource->getIndicesType(),
                            (GLvoid*)0,
                            draw_cnt,
                            0);
                    }

                    //glDisable(GL_CULL_FACE);

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    auto gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error in geometry pass : " << gl_err << std::endl;
                }
                );


                struct LightingPassData
                {
                    Mat4x4 m_view_matrix;
                    Vec2 m_aspect_fovy;
                    float m_exposure;

                    std::vector<Vec4>	m_pointlight_data; ///< vec3 position, float intensity
                    std::vector<Vec4>	m_sunlight_data; ///< vec3 position, float intensity
                };

                struct LightingPassResources
                {
                    WeakResource<GLSLProgram>		m_lighting_prgm;
                    WeakResource<FramebufferObject>	m_gBuffer;
                    WeakResource<Texture2D>			m_target;
                    WeakResource<BufferObject>		m_pointlights_data;
                    WeakResource<BufferObject>		m_sunlights_data;
                };

                frame.addRenderPass<LightingPassData, LightingPassResources>("LightingPass",
                    // data setup phase
                    [&world_state, &resource_mngr](LightingPassData& data, LightingPassResources& resources) {

                    uint camera_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
                    Entity camera_entity = GCoreComponents::cameraManager().getEntity(camera_idx);
                    uint camera_transform_idx = GCoreComponents::transformManager().getIndex(camera_entity);
                    data.m_view_matrix = glm::inverse(GCoreComponents::transformManager().getWorldTransformation(camera_transform_idx));
                    float fovy = GCoreComponents::cameraManager().getFovy(camera_idx);
                    float aspect_ratio = GCoreComponents::cameraManager().getAspectRatio(camera_idx);
                    data.m_aspect_fovy = Vec2();
                    data.m_exposure = GCoreComponents::cameraManager().getExposure(camera_idx);

                    // gather data from lightsource components
                    uint pointlight_cnt = GCoreComponents::pointlightManager().getComponentCount();
                    data.m_pointlight_data.reserve(pointlight_cnt);
                    for (int i = 0; i < pointlight_cnt; i++)
                    {
                        if (/*active*/true)
                        {
                            Vec3 position = GCoreComponents::transformManager().getWorldPosition(GCoreComponents::pointlightManager().getEntity(i));
                            float intensity = GCoreComponents::pointlightManager().getLumen(i);
                            data.m_pointlight_data.push_back(Vec4(position, intensity));
                        }
                    }

                    uint sunlight_cnt = GCoreComponents::sunlightManager().getComponentCount();
                    data.m_sunlight_data.reserve(sunlight_cnt);
                    for (int i = 0; i < sunlight_cnt; i++)
                    {
                        if (/*active*/true)
                        {
                            Vec3 position = GCoreComponents::transformManager().getWorldPosition(GCoreComponents::sunlightManager().getEntity(i));
                            float intensity = GCoreComponents::sunlightManager().getLumen(i);
                            data.m_sunlight_data.push_back(Vec4(position, intensity));
                        }
                    }

                    // try to get resources early
                    resources.m_lighting_prgm = resource_mngr.getGLSLProgram("rendering_pipeline_lighting");
                    resources.m_gBuffer = resource_mngr.getFramebufferObject("gBuffer");
                    resources.m_target = resource_mngr.getTexture("lightingPass_target");
                    resources.m_pointlights_data = resource_mngr.getBufferObject("lightingPass_pointlights_data");
                    resources.m_sunlights_data = resource_mngr.getBufferObject("lightingPass_sunlights_data");
                },
                    // resource setup phase
                    [&world_state, &resource_mngr](LightingPassData& data, LightingPassResources& resources) {
                    if (resources.m_lighting_prgm.state != READY)
                    {
                        resources.m_lighting_prgm = resource_mngr.createShaderProgram({ "../resources/shaders/lighting/BRP_simpleLighting_c.glsl" }, "rendering_pipeline_lighting");
                    }

                    if (resources.m_gBuffer.state != READY)
                        resources.m_gBuffer = resource_mngr.getFramebufferObject("gBuffer");

                    if (resources.m_target.state == READY)
                    {
                        if (resources.m_target.resource->getWidth() != resources.m_gBuffer.resource->getWidth()
                            || resources.m_target.resource->getHeight() != resources.m_gBuffer.resource->getHeight())
                        {
                            TextureLayout lighting_tgt_layout(GL_RGBA32F,
                                resources.m_gBuffer.resource->getWidth(),
                                resources.m_gBuffer.resource->getHeight(),
                                1,
                                GL_RGBA,
                                GL_FLOAT,
                                1,
                                { { GL_TEXTURE_MIN_FILTER, GL_NEAREST },{ GL_TEXTURE_MAG_FILTER, GL_NEAREST } },
                                { {} }
                            );
                            resources.m_target.resource->reload(lighting_tgt_layout, nullptr);
                        }
                    }
                    else
                    {
                        TextureLayout lighting_tgt_layout(GL_RGBA32F,
                            resources.m_gBuffer.resource->getWidth(),
                            resources.m_gBuffer.resource->getHeight(),
                            1,
                            GL_RGBA,
                            GL_FLOAT,
                            1,
                            { { GL_TEXTURE_MIN_FILTER, GL_NEAREST },{ GL_TEXTURE_MAG_FILTER, GL_NEAREST } },
                            { {} }
                        );
                        resources.m_target = resource_mngr.createTexture2D(
                            "lightingPass_target",
                            lighting_tgt_layout,
                            nullptr
                        );
                    }

                    //TODO buffer light data
                    if (resources.m_pointlights_data.state != READY)
                        resources.m_pointlights_data = resource_mngr.createBufferObject("lightingPass_pointlights_data", GL_SHADER_STORAGE_BUFFER, data.m_pointlight_data);
                    else
                        resources.m_pointlights_data.resource->rebuffer(data.m_pointlight_data);

                    if (resources.m_sunlights_data.state != READY)
                        resources.m_sunlights_data = resource_mngr.createBufferObject("lightingPass_sunlights_data", GL_SHADER_STORAGE_BUFFER, data.m_sunlight_data);
                    else
                        resources.m_sunlights_data.resource->rebuffer(data.m_sunlight_data);
                },
                    // execute phase
                    [](LightingPassData const& data, LightingPassResources const& resources) {
                    // set render target etc

                    resources.m_lighting_prgm.resource->use();

                    glActiveTexture(GL_TEXTURE0);
                    resources.m_gBuffer.resource->bindColorbuffer(0);
                    glActiveTexture(GL_TEXTURE1);
                    resources.m_gBuffer.resource->bindColorbuffer(1);
                    glActiveTexture(GL_TEXTURE2);
                    resources.m_gBuffer.resource->bindColorbuffer(2);
                    glActiveTexture(GL_TEXTURE3);
                    resources.m_gBuffer.resource->bindColorbuffer(3);

                    resources.m_lighting_prgm.resource->setUniform("lighting_tx2D", 0);
                    resources.m_target.resource->bindImage(0, GL_WRITE_ONLY);

                    //// Bind textures from framebuffer
                    resources.m_lighting_prgm.resource->setUniform("normal_tx2D", 0);
                    resources.m_lighting_prgm.resource->setUniform("depth_tx2D", 1);
                    resources.m_lighting_prgm.resource->setUniform("albedoRGB_tx2D", 2);
                    resources.m_lighting_prgm.resource->setUniform("specularRGB_roughness_tx2D", 3);

                    //resources.m_lighting_prgm.resource->setUniform("atmosphereRGBA_tx2D", 4);
                    //resources.m_lighting_prgm.resource->setUniform("oceanRGBA_tx2D", 5);

                    //glUniform1i(glGetUniformLocation(resources.m_lighting_prgm.resource->getHandle(), "lighting_tx2D"), 0);
                    //std::cout << glGetUniformLocation(resources.m_lighting_prgm.resource->getHandle(), "view_matrix") << std::endl;

                    // Set information on active camera and screen
                    resources.m_lighting_prgm.resource->setUniform("view_matrix", data.m_view_matrix);
                    resources.m_lighting_prgm.resource->setUniform("aspect_fovy", data.m_aspect_fovy);
                    resources.m_lighting_prgm.resource->setUniform("screen_resolution", Vec2(resources.m_target.resource->getWidth(), resources.m_target.resource->getHeight()));
                    resources.m_lighting_prgm.resource->setUniform("exposure", data.m_exposure);

                    //	//	// TODO Shadows...
                    //	//	glActiveTexture(GL_TEXTURE4);
                    //	//	m_pointlight_shadowmaps->bindTexture();
                    //	//	m_lighting_prgm->setUniform("pointlight_shadowmaps", 4);
                    //	
                    //	resources.m_lighting_prgm.resource->setUniform("num_suns", static_cast<int>(data.m_sunlight_data.size()));

                    //TODO find out why this generates 1282
                    resources.m_lighting_prgm.resource->dispatchCompute(
                        static_cast<GLuint>(std::ceil(static_cast<float>(resources.m_target.resource->getWidth()) / 8.0f)),
                        static_cast<GLuint>(std::ceil(static_cast<float>(resources.m_target.resource->getHeight()) / 8.0f)),
                        1);

                    auto gl_err = glGetError();
                    if (gl_err != GL_NO_ERROR)
                        std::cerr << "GL error in lighting pass : " << gl_err << std::endl;

                    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
                }
                );


                struct CompPassData
                {
                    float m_camera_exposure;
                };
                struct CompPassResources
                {
                    WeakResource<GLSLProgram>		m_compositing_prgm;
                    WeakResource<Texture2D>			m_lighting_target;
                    WeakResource<FramebufferObject>	m_atmosphere_target;
                    WeakResource<FramebufferObject>	m_ocean_target;
                    WeakResource<FramebufferObject>	m_volume_target;
                };
                frame.addRenderPass<CompPassData, CompPassResources>("CompositingPass",
                    // data setup phase
                    [&world_state, &resource_mngr](CompPassData& data, CompPassResources& resources) {
                    // get data
                    uint camera_idx = GCoreComponents::cameraManager().getActiveCameraIndex();
                    data.m_camera_exposure = GCoreComponents::cameraManager().getExposure(camera_idx);

                    // try to get resources early
                    resources.m_compositing_prgm = resource_mngr.getGLSLProgram("rendering_pipeline_compositing");
                    resources.m_lighting_target = resource_mngr.getTexture("lightingPass_target");
                    resources.m_atmosphere_target = resource_mngr.getFramebufferObject("atmosphere_target");
                    resources.m_ocean_target = resource_mngr.getFramebufferObject("ocean_target");
                    resources.m_volume_target = resource_mngr.getFramebufferObject("volume_target");
                },
                    // resource setup phase
                    [&world_state, &resource_mngr](CompPassData& data, CompPassResources& resources) {

                    if (resources.m_compositing_prgm.state != READY)
                    {
                        resources.m_compositing_prgm = resource_mngr.createShaderProgram(
                            { "../resources/shaders/genericPostProc_v.glsl","../resources/shaders/compositing_f.glsl" },
                            "rendering_pipeline_compositing"
                        );
                    }

                    // try to get resources one more time before rendering (is needed during the first frame but pretty useless afterwards...need to think of sth for this)
                    if (resources.m_lighting_target.state != READY) { resources.m_lighting_target = resource_mngr.getTexture("lightingPass_target"); }
                    if (resources.m_atmosphere_target.state != READY) { resources.m_atmosphere_target = resource_mngr.getFramebufferObject("atmosphere_target"); }
                    if (resources.m_ocean_target.state != READY) { resources.m_ocean_target = resource_mngr.getFramebufferObject("ocean_target"); }
                    if (resources.m_volume_target.state != READY) { resources.m_volume_target = resource_mngr.getFramebufferObject("volume_target"); }
                },
                    // execute phase
                    [](CompPassData const& data, CompPassResources const& resources) {
                    // do tone-mapping, i.e. gamma correction in the most simple case

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    auto window_resolution = GEngineCore::graphicsBackend().getActiveWindowResolution();
                    glViewport(0, 0, std::get<0>(window_resolution), std::get<1>(window_resolution));

                    resources.m_compositing_prgm.resource->use();

                    glActiveTexture(GL_TEXTURE0);
                    resources.m_compositing_prgm.resource->setUniform("lighting_tx2D", 0);
                    resources.m_lighting_target.resource->bindTexture();

                    /*
                    glActiveTexture(GL_TEXTURE1);
                    resources.m_compositing_prgm.resource->setUniform("atmosphere_tx2D", 1);
                    resources.m_atmosphere_target.resource->bindColorbuffer(0);

                    glActiveTexture(GL_TEXTURE2);
                    resources.m_compositing_prgm.resource->setUniform("ocean_tx2D", 2);
                    resources.m_ocean_target.resource->bindColorbuffer(0);

                    glActiveTexture(GL_TEXTURE3);
                    resources.m_compositing_prgm.resource->setUniform("volume_tx2D", 3);
                    resources.m_volume_target.resource->bindColorbuffer(0);
                    */

                    resources.m_compositing_prgm.resource->setUniform("exposure", data.m_camera_exposure);

                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
                );


                //struct InterfacePassData
                //{
                //	struct InterfaceMeshParams
                //	{
                //		Mat4x4		transform;
                //		uint32_t	selected;
                //	};
                //
                //	std::vector<InterfaceMeshParams>	interface_mesh_params;
                //};

                struct InterfacePassData
                {
                };

                struct InterfacePassResources
                {
                };

                frame.addRenderPass<InterfacePassData, InterfacePassData>("InterfacePass",
                    [](InterfacePassData& data, InterfacePassData& resources) {},
                    [](InterfacePassData& data, InterfacePassData& resources) {
                    ImGui_ImplGlfwGL3_NewFrame();
                },
                    [](InterfacePassData const& data, InterfacePassData const& resources) {
                    auto res = GEngineCore::graphicsBackend().getActiveWindowResolution();
                    ImGui::SetNextWindowPos(ImVec2(std::get<0>(res) - 375.0f, std::get<1>(res) - 200.0f));
                    bool p_open = true;
                    if (!ImGui::Begin("", &p_open, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
                    {
                        ImGui::End();
                        return;
                    }

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::End();
                    ImGui::Render();
                }
                );
            }

        }
    }
}

#endif //  0