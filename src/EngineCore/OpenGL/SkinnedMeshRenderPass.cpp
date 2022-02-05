#include "SkinnedMeshRenderPass.hpp"

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>

#include "CameraComponent.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"
#include "SkinComponentManager.hpp"
#include "TransformComponentManager.hpp"

void EngineCore::Graphics::OpenGL::addSkinnedMeshRenderPass(Common::Frame& frame, WorldState& world_state, ResourceManager& resource_mngr)
{
    struct SkinnedMeshPassData
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

        struct SkinnedMeshParams
        {
            Mat4x4 transform;

            GLint joint_index_offset;

            GLint padding0;
            GLint padding1;
            GLint padding2;
        };

        // static mesh (shader) params per object per batch
        std::vector<std::vector<SkinnedMeshParams>>   skinned_mesh_params;
        std::vector<std::vector<DrawElementsCommand>> skinned_mesh_drawCommands;

        std::vector<Mat4x4> joint_matrices;

        Mat4x4 view_matrix;
        Mat4x4 proj_matrix;
    };

    struct SkinnedMeshPassResources {
        struct BatchResources
        {
            WeakResource<glowl::GLSLProgram>  shader_prgm;
            WeakResource<glowl::BufferObject> object_params;
            WeakResource<glowl::BufferObject> draw_commands;
            WeakResource<glowl::Mesh>         geometry;
        };

        std::vector<BatchResources> m_batch_resources;

        WeakResource<glowl::BufferObject> joint_matrices;

        WeakResource<glowl::FramebufferObject> m_render_target;
    };

    frame.addRenderPass<SkinnedMeshPassData, SkinnedMeshPassResources>("SkinnedMeshPass",
        [&frame, &world_state, &resource_mngr](SkinnedMeshPassData& data, SkinnedMeshPassResources& resources)
        {
            auto& cam_mngr = world_state.get<CameraComponentManager>();
            auto& mtl_mngr = world_state.get<MaterialComponentManager<ResourceManager>>();
            auto& mesh_mngr = world_state.get<MeshComponentManager<ResourceManager>>();
            auto& renderTask_mngr = world_state.get<RenderTaskComponentManager<Graphics::RenderTaskTags::SkinnedMesh>>();
            auto& skin_mngr = world_state.get<Animation::SkinComponentManager>();
            auto& transform_mngr = world_state.get<Common::TransformComponentManager>();

            // set camera matrices
            Entity camera_entity = cam_mngr.getActiveCamera();
            auto camera_idx = cam_mngr.getIndex(camera_entity).front();
            auto camera_transform_idx = transform_mngr.getIndex(camera_entity);

            data.view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));

            if (frame.m_window_width != 0 && frame.m_window_height != 0) {
                cam_mngr.setAspectRatio(camera_idx, static_cast<float>(frame.m_window_width) / static_cast<float>(frame.m_window_height));
                cam_mngr.updateProjectionMatrix(camera_idx);
                data.proj_matrix = cam_mngr.getProjectionMatrix(camera_idx);
            }

            // check for existing gBuffer
            resources.m_render_target = resource_mngr.getFramebufferObject("GBuffer");

            // check for existing joint matrix buffer
            resources.joint_matrices = resource_mngr.getBufferResource("skinnedMeshPass_joint_matrices_" + std::to_string(frame.m_frameID % 2));

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

                    auto batch_idx = data.skinned_mesh_params.size();
                    data.skinned_mesh_params.push_back(std::vector<SkinnedMeshPassData::SkinnedMeshParams>());
                    data.skinned_mesh_drawCommands.push_back(std::vector<SkinnedMeshPassData::DrawElementsCommand>());

                    // TODO query batch GPU resources early?
                    SkinnedMeshPassResources::BatchResources batch_resources;
                    batch_resources.shader_prgm = resource_mngr.getShaderProgramResource(current_prgm);
                    batch_resources.object_params = resource_mngr.getBufferResource("skinnedMeshPass_obj_params_" + std::to_string(batch_idx) + "_" + std::to_string(frame.m_frameID % 2));
                    batch_resources.draw_commands = resource_mngr.getBufferResource("skinnedMeshPass_draw_commands_" + std::to_string(batch_idx) + "_" + std::to_string(frame.m_frameID % 2));
                    batch_resources.geometry = resource_mngr.getMeshResource(current_mesh);
                    resources.m_batch_resources.push_back(batch_resources);
                }

                // gather all per object information
                SkinnedMeshPassData::SkinnedMeshParams params;

                params.transform = transform_mngr.getWorldTransformation(obj.cached_transform_idx);
                params.joint_index_offset = data.joint_matrices.size();

                //compute joint matrices
                auto const& joints = skin_mngr.getJoints(obj.entity);
                auto const& inverse_bind_matrices = skin_mngr.getInvsereBindMatrices(obj.entity);
                for (int i = 0; i < joints.size(); ++i) {
                    auto const& joint_world_matrix = transform_mngr.getWorldTransformation(transform_mngr.getIndex(joints[i]));
                    auto const& inverse_bind_matrix = inverse_bind_matrices[i];

                    Mat4x4 joint_matrix = glm::inverse(params.transform) * joint_world_matrix * inverse_bind_matrix;
                    data.joint_matrices.push_back(joint_matrix);
                }

                data.skinned_mesh_params.back().push_back(params);

                // set draw command values
                SkinnedMeshPassData::DrawElementsCommand draw_command;
                auto draw_params = mesh_mngr.getDrawIndexedParams(obj.cached_mesh_idx);
                draw_command.cnt = std::get<0>(draw_params);
                draw_command.base_vertex = std::get<2>(draw_params);
                draw_command.first_idx = std::get<1>(draw_params);
                draw_command.base_instance = 0;
                draw_command.instance_cnt = 1;
                data.skinned_mesh_drawCommands.back().push_back(draw_command);
            }

        },
        [&frame, &world_state, &resource_mngr](SkinnedMeshPassData& data, SkinnedMeshPassResources& resources)
        {
            if (resources.m_render_target.state != READY)
            {
                // check for existing gBuffer
                resources.m_render_target = resource_mngr.getFramebufferObject("GBuffer");
            }

            if (resources.joint_matrices.state != READY)
            {
                resources.joint_matrices = resource_mngr.createBufferObject(
                    "skinnedMeshPass_joint_matrices_" + std::to_string(frame.m_frameID % 2),
                    GL_SHADER_STORAGE_BUFFER,
                    data.joint_matrices
                );
            }
            else
            {
                try
                {
                    resources.joint_matrices.resource->rebuffer(data.joint_matrices);
                }
                catch (glowl::BufferObjectException const& e)
                {
                    std::cerr << "Exception in skinned mesh pass resource setup - joint matrix rebuffer: " << e.what() << std::endl;
                }
            }

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
                    batch_resources.object_params = resource_mngr.createBufferObject(
                        "skinnedMeshPass_obj_params_" + std::to_string(batch) + "_" + std::to_string(frame.m_frameID % 2),
                        GL_SHADER_STORAGE_BUFFER,
                        data.skinned_mesh_params[batch]
                    );
                }
                else
                {
                    try
                    {
                        batch_resources.object_params.resource->rebuffer(data.skinned_mesh_params[batch]);
                    }
                    catch (glowl::BufferObjectException const& e)
                    {
                        std::cerr << "Exception in skinned mesh pass resource setup - batch " << batch << " : " << e.what() << std::endl;
                    }
                }

                if (batch_resources.draw_commands.state != READY)
                {
                    batch_resources.draw_commands = resource_mngr.createBufferObject(
                        "skinnedMeshPass_draw_commands_" + std::to_string(batch) + "_" + std::to_string(frame.m_frameID % 2),
                        GL_DRAW_INDIRECT_BUFFER,
                        data.skinned_mesh_drawCommands[batch]
                    );
                }
                else
                {
                    try
                    {
                        //batch_resources.draw_commands.resource->rebuffer(data.static_mesh_drawCommands[batch]);
                        batch_resources.draw_commands.resource->rebuffer(data.skinned_mesh_drawCommands[batch]);
                    }
                    catch (glowl::BufferObjectException const& e)
                    {
                        std::cerr << "Exception in skinned mesh pass resource setup - batch " << batch << " : " << e.what() << std::endl;
                    }
                }

                ++batch;

                auto gl_err = glGetError();
                if (gl_err != GL_NO_ERROR)
                    std::cerr << "GL error in skinned mesh pass resource setup - batch " << batch << " : " << gl_err << std::endl;
            }
        },
        [&frame, &world_state, &resource_mngr](SkinnedMeshPassData const& data, SkinnedMeshPassResources const& resources)
        {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glEnable(GL_DEPTH_TEST);
        
            resources.m_render_target.resource->bind();
            int width = resources.m_render_target.resource->getWidth();
            int height = resources.m_render_target.resource->getHeight();
            glViewport(0, 0, width, height);
        
            auto gl_err = glGetError();
            if (gl_err != GL_NO_ERROR)
                std::cerr << "GL error in skinned mesh pass execution: " << gl_err << std::endl;
        
            uint batch_idx = 0;
            for (auto& batch_resources : resources.m_batch_resources)
            {
                if (batch_resources.shader_prgm.state != READY || batch_resources.geometry.state != READY)
                    continue;
        
                batch_resources.shader_prgm.resource->use();
        
                batch_resources.shader_prgm.resource->setUniform("view_matrix", data.view_matrix);
                batch_resources.shader_prgm.resource->setUniform("projection_matrix", data.proj_matrix);
        
                batch_resources.object_params.resource->bind(0);

                resources.joint_matrices.resource->bind(1);

                batch_resources.draw_commands.resource->bind();
                batch_resources.geometry.resource->bindVertexArray();
        
                GLsizei draw_cnt = static_cast<GLsizei>(data.skinned_mesh_drawCommands[batch_idx].size());
        
                glMultiDrawElementsIndirect(
                    batch_resources.geometry.resource->getPrimitiveType(),
                    batch_resources.geometry.resource->getIndexType(),
                    (GLvoid*)0,
                    draw_cnt,
                    0);
        
                auto gl_err = glGetError();
                if (gl_err != GL_NO_ERROR)
                    std::cerr << "GL error in skinned mesh pass execution - batch " << batch_idx << " : " << gl_err << std::endl;
        
                ++batch_idx;
            }
        
            glBindVertexArray(0);
        
            //glDisable(GL_CULL_FACE);
        
            //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
            //ImGui::SetNextWindowPos(ImVec2(frame.m_window_width - 375.0f, frame.m_window_height - 135.0f));
            //bool p_open = true;
            //if (!ImGui::Begin("Render Stats", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
            //{
            //    ImGui::End();
            //    return;
            //}
            //ImGui::Text("# batches (draw calls): %u ", batch_idx);
            //ImGui::End();
        }
    );

}
