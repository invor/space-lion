#include "LandscapeSystems.hpp"

#include "LandscapeFeatureCurveComponent.hpp"
#include "TransformComponentManager.hpp"

namespace EngineCore {
namespace Graphics {
namespace Landscape {
namespace OpenGL {

    namespace {
        void resetFields(
            Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            // Reset head buffer and surface texture
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            // get all OpenGL resources
            auto reset_prgm = resource_mngr.createShaderProgram("lcsp_reset", { {"../resources/shaders/landscape/voxelization_reset_c.glsl",glowl::GLSLProgram::ShaderType::Compute} });
            auto head = resource_mngr.getBufferResource(brick.m_head);
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto surface_backbuffer = resource_mngr.getTexture3DResource(brick.m_surface_backbuffer);
            auto noise_params = resource_mngr.getTexture3DResource(brick.m_noise_params);
            auto gradients = resource_mngr.getTexture3DResource(brick.m_gradients);

            if (reset_prgm.state == READY && 
                head.state == READY && 
                surface.state == READY && 
                surface_backbuffer.state == READY && 
                noise_params.state == READY)
            {
                reset_prgm.resource->use();

                reset_prgm.resource->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
                reset_prgm.resource->setUniform("cell_size", brick.m_dimensions / Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
                head.resource->bind(0);

                surface.resource->bindImage(0, GL_WRITE_ONLY);
                reset_prgm.resource->setUniform("surface_tx3D", 0);
                surface_backbuffer.resource->bindImage(1, GL_WRITE_ONLY);
                reset_prgm.resource->setUniform("surface_backbuffer_tx3D", 1);
                noise_params.resource->bindImage(2, GL_WRITE_ONLY);
                reset_prgm.resource->setUniform("noise_tx3D", 2);

                glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

                //sm_bricks[brick_idx].m_gradients->bindTexture();
                glClearTexImage(gradients.resource->getName(), 0, GL_RGBA, GL_FLOAT, NULL);

                //glClearTexImage(brick.m_noise_params->getName(), 0, GL_RGBA, GL_HALF_FLOAT, NULL);

                /*	Reset the atomic counter */
                GLuint zero = 0;
                glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, brick.m_counter_buffer);
                glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
                glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
                glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, 0);
            }

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Reset fields - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
        }

        void voxelizeFeatureCurves(
            FeatureCurveComponentManager<Graphics::OpenGL::ResourceManager>& feature_curve_mngr,
            Graphics::OpenGL::ResourceManager& resource_mngr,
            Common::TransformComponentManager const& transfrom_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            auto voxelize_prgm = resource_mngr.createShaderProgram("lcsp_voxelize", { {"../resources/shaders/landscape/voxelization_gather_c.glsl",glowl::GLSLProgram::ShaderType::Compute} }).resource;
            auto head = resource_mngr.getBufferResource(brick.m_head);
            auto guidancefield = resource_mngr.getBufferResource(brick.m_guidancefield_data);
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto surface_backbuffer = resource_mngr.getTexture3DResource(brick.m_surface_backbuffer);
            auto noise_params = resource_mngr.getTexture3DResource(brick.m_noise_params);

            // Voxelize geometry
            voxelize_prgm->use();

            head.resource->bind(2);
            guidancefield.resource->bind(3);
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, brick.m_counter_buffer);

            surface.resource->bindImage(0, GL_WRITE_ONLY);
            voxelize_prgm->setUniform("surface_tx3D", 0);
            surface_backbuffer.resource->bindImage(1, GL_WRITE_ONLY);
            voxelize_prgm->setUniform("surface_backbuffer_tx3D", 1);

            noise_params.resource->bindImage(2, GL_WRITE_ONLY);
            voxelize_prgm->setUniform("noise_tx3D", 2);

            // Compute voxel transform matrix
            Vec3 brick_position = transfrom_mngr.getPosition(transfrom_mngr.getIndex(brick.m_entity));
            Vec3 brick_dimension = brick.m_dimensions;
            //	Mat4x4 voxel_matrix = glm::ortho(brick_position.x - brick_dimension.x/2.0f,
            //										brick_position.x + brick_dimension.x/2.0f,
            //										brick_position.y - brick_dimension.y/2.0f,
            //										brick_position.y + brick_dimension.y/2.0f,
            //										brick_position.z + brick_dimension.z/2.0f,
            //										brick_position.z - brick_dimension.z/2.0f);

            //voxel_matrix = glm::translate(Mat4x4(), -1.0f * Vec3( brick_position.x, brick_position.y, brick_position.z));
            //voxel_matrix = glm::inverse(transfrom_mngr.getWorldTransformation(transfrom_mngr.getIndex(brick.m_entity)));
            Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0 / brick_dimension.x, 1.0f / brick_dimension.y, 1.0f / brick_dimension.z));
            voxel_matrix = voxel_matrix * glm::inverse(transfrom_mngr.getWorldTransformation(transfrom_mngr.getIndex(brick.m_entity)));
            voxelize_prgm->setUniform("voxel_matrix", voxel_matrix);

            voxelize_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));

            voxelize_prgm->setUniform("cell_size", Vec3(brick.m_dimensions.x / brick.m_res_x,
                brick.m_dimensions.y / brick.m_res_y,
                brick.m_dimensions.z / brick.m_res_z));

            std::cout << "Feature Curves in Brick: " << brick.m_featureCurves.size() << std::endl;

            for (auto& featureCurve : brick.m_featureCurves)
            {
                uint featureCurve_idx = feature_curve_mngr.getIndex(featureCurve).second;

                // get feature curve model matrix
                auto model_matrix = transfrom_mngr.getWorldTransformation(transfrom_mngr.getIndex(featureCurve));
                voxelize_prgm->setUniform("model_matrix", model_matrix);

                auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
                voxelize_prgm->setUniform("normal_matrix", normal_matrix);

                auto proxy_mesh = resource_mngr.getMeshResource(feature_curve_mngr.getProxyMesh(featureCurve_idx));

                voxelize_prgm->setUniform("num_triangles", static_cast<unsigned int>(proxy_mesh.resource->getIbo().getByteSize() / 12)); // ssbo size is in bytes, therefore /4 and /3 to get triangle count
                voxelize_prgm->setUniform("is_surface_seed", feature_curve_mngr.isSurfaceSeed(featureCurve_idx));

                proxy_mesh.resource->getVbos().front()->bindAs(GL_SHADER_STORAGE_BUFFER, 0);
                proxy_mesh.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

                glDispatchCompute(static_cast<uint>(std::ceil(static_cast<float>(proxy_mesh.resource->getIbo().getByteSize() / 12) / 32.0)), 1, 1);

                std::cout << "Triangles: " << proxy_mesh.resource->getIbo().getByteSize() / 12 << std::endl;

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
            }


            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Voxelize Feature Curves - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

            /*
            Entity vox_vol = GEngineCore::entityManager().create();
            transfrom_mngr.addComponent(vox_vol, Vec3(-128.0,0.0,0.0), Quat(), Vec3(64, 32, 64));
            std::shared_ptr<Mesh> bb = resource_mngr.createBox();

            std::shared_ptr<glowl::Texture3D> out_vol = resource_mngr.createTexture3D("brick_" + std::to_string(index) + "_normals",
                                                                                            GL_RGBA32F,
                                                                                            brick.m_res_x,
                                                                                            brick.m_res_y,
                                                                                            brick.m_res_z,
                                                                                            GL_RGBA, GL_FLOAT, nullptr);

            GRenderingComponents::volumeManager().addComponent(vox_vol, out_vol, bb, Vec3(), Vec3(64.0,32.0,64.0));
            */
        }

        // TODO port old implementation
        void voxelizeFeatureMeshes(
            Graphics::OpenGL::ResourceManager& resource_mngr,
            Common::TransformComponentManager const& transfrom_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            //  GLuint64 t_0, t_1;
            //  unsigned int queryID[2];
            //  // generate two queries
            //  glGenQueries(2, queryID);
            //  glQueryCounter(queryID[0], GL_TIMESTAMP);
            //  
            //  glMemoryBarrier(GL_ALL_BARRIER_BITS);
            //  
            //  auto voxelize_mesh_prgm = resource_mngr.createShaderProgram("lcsp_voxelize_mesh", { { "../resources/shaders/landscape/voxelize_mesh_gather_c.glsl",glowl::GLSLProgram::ShaderType::Compute } }).resource;
            //  auto head = resource_mngr.getBufferResource(brick.m_head);
            //  auto guidancefield = resource_mngr.getBufferResource(brick.m_guidancefield_data);
            //  auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            //  auto surface_backbuffer = resource_mngr.getTexture3DResource(brick.m_surface_backbuffer);
            //  auto noise_params = resource_mngr.getTexture3DResource(brick.m_noise_params);
            //  
            //  // Voxelize geometry
            //  voxelize_mesh_prgm->use();
            //  
            //  head.resource->bind(2);
            //  guidancefield.resource->bind(3);
            //  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, brick.m_counter_buffer);
            //  
            //  surface.resource->bindImage(0, GL_WRITE_ONLY);
            //  voxelize_mesh_prgm->setUniform("surface_tx3D", 0);
            //  surface_backbuffer.resource->bindImage(1, GL_WRITE_ONLY);
            //  voxelize_mesh_prgm->setUniform("surface_backbuffer_tx3D", 1);
            //  
            //  noise_params.resource->bindImage(2, GL_READ_WRITE);
            //  voxelize_mesh_prgm->setUniform("noise_tx3D", 2);
            //  
            //  // Compute voxel transform matrix
            //  Vec3 brick_position = transfrom_mngr.getPosition(transfrom_mngr.getIndex(brick.m_entity));
            //  Vec3 brick_dimension = brick.m_dimensions;
            //  
            //  Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0 / brick_dimension.x, 1.0f / brick_dimension.y, 1.0f / brick_dimension.z));
            //  voxel_matrix = voxel_matrix * glm::inverse(transfrom_mngr.getWorldTransformation(transfrom_mngr.getIndex(brick.m_entity)));
            //  voxelize_mesh_prgm->setUniform("voxel_matrix", voxel_matrix);
            //  
            //  voxelize_mesh_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            //  
            //  voxelize_mesh_prgm->setUniform("cell_size", Vec3(
            //      brick.m_dimensions.x / brick.m_res_x,
            //      brick.m_dimensions.y / brick.m_res_y,
            //      brick.m_dimensions.z / brick.m_res_z)
            //  );
            //  
            //  std::cout << "Feature Meshes in Brick: " << brick.m_featureMeshes.size() << std::endl;
            //  
            //  for (auto& featureMesh : brick.m_featureMeshes)
            //  {
            //      uint featureMesh_idx = GLandscapeComponents::featureMeshManager().getIndex(featureMesh);
            //  
            //      // get feature curve model matrix
            //      auto model_matrix = transfrom_mngr.getWorldTransformation(transfrom_mngr.getIndex(featureMesh));
            //      voxelize_mesh_prgm->setUniform("model_matrix", model_matrix);
            //  
            //      auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
            //      voxelize_mesh_prgm->setUniform("normal_matrix", normal_matrix);
            //  
            //      auto mesh = GLandscapeComponents::featureMeshManager().getMesh(featureMesh_idx);
            //  
            //      if (mesh == nullptr)
            //          continue;
            //  
            //      voxelize_mesh_prgm->setUniform("num_triangles", (mesh->getIndicesCount() / 3));
            //      voxelize_mesh_prgm->setUniform("is_surface_seed", true);
            //  
            //      // Bind vertex and index buffer as storage buffer
            //      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->getVboHandle());
            //      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->getIboHandle());
            //      mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
            //      mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
            //  
            //      glDispatchCompute(static_cast<uint>(std::ceil(static_cast<float>((mesh->getIndicesCount() / 3) / 32.0))), 1, 1);
            //  
            //      std::cout << "Triangles: " << (mesh->getIndicesCount() / 3) << std::endl;
            //  
            //      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            //  
            //      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
            //      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
            //  }
            //  
            //  
            //  glQueryCounter(queryID[1], GL_TIMESTAMP);
            //  
            //  // wait until the results are available
            //  GLint stopTimerAvailable = 0;
            //  while (!stopTimerAvailable)
            //      glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
            //  
            //  // get query results
            //  glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            //  glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);
            //  
            //  std::cout << "Voxelize Feature Mesh - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
        }

        // TODO port old implementation
        void voxelizeHeightmapMeshes(uint index)
        {
            //  GLuint64 t_0, t_1;
            //  unsigned int queryID[2];
            //  // generate two queries
            //  glGenQueries(2, queryID);
            //  glQueryCounter(queryID[0], GL_TIMESTAMP);
            //  
            //  glMemoryBarrier(GL_ALL_BARRIER_BITS);
            //  
            //  
            //  // Voxelize geometry
            //  voxelize_heightmapMesh_prgm->use();
            //  
            //  brick.m_head->bind(2);
            //  brick.m_guidancefield_data->bind(3);
            //  glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, brick.m_counter_buffer);
            //  
            //  brick.m_surface->bindImage(0, GL_WRITE_ONLY);
            //  voxelize_heightmapMesh_prgm->setUniform("surface_tx3D", 0);
            //  brick.m_surface_backbuffer->bindImage(1, GL_WRITE_ONLY);
            //  voxelize_heightmapMesh_prgm->setUniform("surface_backbuffer_tx3D", 1);
            //  
            //  brick.m_noise_params->bindImage(2, GL_READ_WRITE);
            //  voxelize_heightmapMesh_prgm->setUniform("noise_tx3D", 2);
            //  
            //  // Compute voxel transform matrix
            //  Vec3 brick_position = GCoreComponents::transformManager().getPosition(GCoreComponents::transformManager().getIndex(brick.m_entity));
            //  Vec3 brick_dimension = brick.m_dimensions;
            //  
            //  Mat4x4 voxel_matrix = glm::scale(Mat4x4(), Vec3(1.0 / brick_dimension.x, 1.0f / brick_dimension.y, 1.0f / brick_dimension.z));
            //  voxel_matrix = voxel_matrix * glm::inverse(GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(brick.m_entity)));
            //  voxelize_heightmapMesh_prgm->setUniform("voxel_matrix", voxel_matrix);
            //  
            //  voxelize_heightmapMesh_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            //  
            //  voxelize_heightmapMesh_prgm->setUniform("cell_size", Vec3(brick.m_dimensions.x / brick.m_res_x,
            //      brick.m_dimensions.y / brick.m_res_y,
            //      brick.m_dimensions.z / brick.m_res_z));
            //  
            //  std::cout << "Feature Meshes in Brick: " << brick.m_heightmaps.size() << std::endl;
            //  
            //  for (auto& heightmap : brick.m_heightmaps)
            //  {
            //      uint featureMesh_idx = GLandscapeComponents::featureMeshManager().getIndex(heightmap);
            //  
            //      // get feature curve model matrix
            //      auto model_matrix = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(heightmap));
            //      voxelize_heightmapMesh_prgm->setUniform("model_matrix", model_matrix);
            //  
            //      auto normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
            //      voxelize_heightmapMesh_prgm->setUniform("normal_matrix", normal_matrix);
            //  
            //      auto mesh = GLandscapeComponents::featureMeshManager().getMesh(featureMesh_idx);
            //  
            //      if (mesh == nullptr)
            //          continue;
            //  
            //      voxelize_heightmapMesh_prgm->setUniform("num_triangles", (mesh->getIndicesCount() / 3));
            //      voxelize_heightmapMesh_prgm->setUniform("is_surface_seed", true);
            //  
            //      // Bind vertex and index buffer as storage buffer
            //      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->getVboHandle());
            //      //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->getIboHandle());
            //      mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
            //      mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
            //  
            //      voxelize_heightmapMesh_prgm->dispatchCompute(static_cast<uint>(std::ceil(static_cast<float>((mesh->getIndicesCount() / 3) / 32.0))), 1, 1);
            //  
            //      std::cout << "Triangles: " << (mesh->getIndicesCount() / 3) << std::endl;
            //  
            //      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            //  
            //      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
            //      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
            //  }
            //  
            //  
            //  glQueryCounter(queryID[1], GL_TIMESTAMP);
            //  
            //  // wait until the results are available
            //  GLint stopTimerAvailable = 0;
            //  while (!stopTimerAvailable)
            //      glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
            //  
            //  // get query results
            //  glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            //  glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);
            //  
            //  std::cout << "Voxelize Heightmap Meshes - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
        }

        void averageVoxelization(
            Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            auto average_prgm = resource_mngr.createShaderProgram("lcsp_voxelize_average", {{ "../resources/shaders/landscape/voxelization_average_c.glsl",glowl::GLSLProgram::ShaderType::Compute }}).resource;
            auto head = resource_mngr.getBufferResource(brick.m_head);
            auto guidancefield = resource_mngr.getBufferResource(brick.m_guidancefield_data);
            auto normals = resource_mngr.getTexture3DResource(brick.m_normals);
            auto normals_backbuffer = resource_mngr.getTexture3DResource(brick.m_normals_backbuffer);
            auto gradients = resource_mngr.getTexture3DResource(brick.m_gradients);
            auto gradients_backbuffer = resource_mngr.getTexture3DResource(brick.m_gradients_backbuffer);
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto surface_backbuffer = resource_mngr.getTexture3DResource(brick.m_surface_backbuffer);
            auto noise_params = resource_mngr.getTexture3DResource(brick.m_noise_params);

            // Average the gathered vectors
            average_prgm->use();
            average_prgm->setUniform("grid_size", Vec3(brick.m_res_x,brick.m_res_y, brick.m_res_z));
            head.resource->bind(0);
            guidancefield.resource->bind(1);

            normals.resource->bindImage(0, GL_WRITE_ONLY);
            average_prgm->setUniform("normals_tx3D", 0);
            normals_backbuffer.resource->bindImage(1, GL_WRITE_ONLY);
            average_prgm->setUniform("normals_backbuffer_tx3D", 1);

            gradients.resource->bindImage(2, GL_WRITE_ONLY);
            average_prgm->setUniform("gradient_tx3D", 2);
            gradients_backbuffer.resource->bindImage(3, GL_WRITE_ONLY);
            average_prgm->setUniform("gradient_backbuffer_tx3D", 3);

            surface.resource->bindImage(4, GL_WRITE_ONLY);
            average_prgm->setUniform("surface_tx3D", 4);
            surface_backbuffer.resource->bindImage(5, GL_WRITE_ONLY);
            average_prgm->setUniform("surface_backbuffer_tx3D", 5);

            noise_params.resource->bindImage(6, GL_READ_WRITE);
            average_prgm->setUniform("noise_tx3D", 6);

            glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);
        }

        void computeGuidanceField(
            Graphics::OpenGL::ResourceManager& resource_mngr, 
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick,
            uint iterations = 0)
        {
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            // get all OpenGL resources
            auto buildGuidance_prgm = resource_mngr.createShaderProgram("lcsp_buildGuidance", { { "../resources/shaders/landscape/buildGuidanceField_c.glsl" , glowl::GLSLProgram::ShaderType::Compute} }).resource;
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto eastern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_eastern_boundary[0]);
            auto western_boundary_0 = resource_mngr.getTexture3DResource(brick.m_western_boundary[0]);
            auto upper_boundary_0 = resource_mngr.getTexture3DResource(brick.m_upper_boundary[0]);
            auto lower_boundary_0 = resource_mngr.getTexture3DResource(brick.m_lower_boundary[0]);
            auto nothern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_northern_boundary[0]);
            auto southern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_southern_boundary[0]);
            auto normals = resource_mngr.getTexture3DResource(brick.m_normals);
            auto normals_backbuffer = resource_mngr.getTexture3DResource(brick.m_normals_backbuffer);

            // TODO set textures of current brick to mirrored repeat

            buildGuidance_prgm->use();

            buildGuidance_prgm->setUniform("src_tx3D", 0);
            buildGuidance_prgm->setUniform("tgt_tx3D", 1);

            surface.resource->bindImage(2, GL_READ_ONLY);
            buildGuidance_prgm->setUniform("bc_tx3D", 2);

            buildGuidance_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));

            // set boundary textures
            glActiveTexture(GL_TEXTURE0);
            eastern_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("east_tx3D", 0);
            glActiveTexture(GL_TEXTURE1);
            western_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("west_tx3D", 1);
            glActiveTexture(GL_TEXTURE2);
            upper_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("up_tx3D", 2);
            glActiveTexture(GL_TEXTURE3);
            lower_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("down_tx3D", 3);
            glActiveTexture(GL_TEXTURE4);
            nothern_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("north_tx3D", 4);
            glActiveTexture(GL_TEXTURE5);
            southern_boundary_0.resource->bindTexture();
            buildGuidance_prgm->setUniform("south_tx3D", 5);

            int src = 0;

            // adapt iterations to ~1.5 times grid size
            if (iterations == 0)
                iterations = static_cast<int>(1.5f * std::max(
                        std::max(
                            normals.resource->getWidth(),
                            normals.resource->getHeight()
                        ),
                        normals.resource->getDepth())
                    );
            //iterations = static_cast<int>(2.5f * std::max(std::max(brick.m_normals->getWidth(), brick.m_normals->getHeight()), brick.m_normals->getDepth()));

            for (uint i = 0; i < iterations; i++)
            {
                normals.resource->bindImage(src, GL_READ_ONLY);
                normals_backbuffer.resource->bindImage((src + 1) % 2, GL_WRITE_ONLY);

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                glDispatchCompute(static_cast<uint>(std::floor(normals.resource->getWidth() / 4)) + 1,
                    static_cast<uint>(std::floor(normals.resource->getHeight() / 4)) + 1,
                    static_cast<uint>(std::floor(normals.resource->getDepth() / 4)) + 1);


                src = (src == 0) ? 1 : 0;
            }

            // TODO reset textures of current brick to repeat

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Guidance field diffusion - " << iterations << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

            /*
            auto buildGuidanceGradient_prgm = resource_mngr.createShaderProgram({ "../resources/shaders/landscape/buildGuidanceField_gradients_c.glsl" });

            buildGuidanceGradient_prgm->use();

            buildGuidanceGradient_prgm->setUniform("src_tx3D", 0);
            buildGuidanceGradient_prgm->setUniform("tgt_tx3D", 1);

            brick.m_surface->bindImage(2, GL_READ_ONLY);
            buildGuidanceGradient_prgm->setUniform("bc_tx3D", 2);

            brick.m_normals->bindImage(3, GL_READ_ONLY);
            buildGuidanceGradient_prgm->setUniform("normals_tx3D", 3);

            buildGuidanceGradient_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));

            src = 0;

            for (int i = 1; i<128; i++)
            {
                glBindImageTexture(src, brick.m_gradients->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glBindImageTexture((src + 1) % 2, brick.m_gradients_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                glTextureBarrierNV();


                buildGuidanceGradient_prgm->dispatchCompute(brick.m_gradients->getWidth(), brick.m_gradients->getHeight(), brick.m_gradients->getDepth());


                src = (src == 0) ? 1 : 0;
            }
            */
        }

        void computeNoiseField(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick,
            uint iterations = 0)
        {
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            glowl::TextureLayout rgba16f_layout(GL_RGBA16F, brick.m_res_x, brick.m_res_y, brick.m_res_z, GL_RGBA, GL_HALF_FLOAT, 1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
                    std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});

            glowl::Texture3D backbuffer("noise_backbuffer", rgba16f_layout, nullptr);

            auto buildNoiseField_prgm = resource_mngr.createShaderProgram("lcsp_buildNoiseField", {{ "../resources/shaders/landscape/buildNoiseField_c.glsl", glowl::GLSLProgram::ShaderType::Compute}}).resource;
            auto copyNoiseField_prgm = resource_mngr.createShaderProgram("lcsp_copyNoiseField",{{ "../resources/shaders/copyTexture3D_RGBA16_c.glsl", glowl::GLSLProgram::ShaderType::Compute}}).resource;
            auto noise_params = resource_mngr.getTexture3DResource(brick.m_noise_params);
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);

            copyNoiseField_prgm->use();
            copyNoiseField_prgm->setUniform("src_tx3D", 0);
            copyNoiseField_prgm->setUniform("tgt_tx3D", 1);
            copyNoiseField_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            noise_params.resource->bindImage(0, GL_READ_ONLY);
            backbuffer.bindImage(1, GL_WRITE_ONLY);
            glDispatchCompute(static_cast<uint>(std::floor(noise_params.resource->getWidth() / 4)) + 1,
                static_cast<uint>(std::floor(noise_params.resource->getHeight() / 2)) + 1,
                static_cast<uint>(std::floor(noise_params.resource->getDepth() / 4)) + 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            buildNoiseField_prgm->use();

            buildNoiseField_prgm->setUniform("src_tx3D", 0);
            buildNoiseField_prgm->setUniform("tgt_tx3D", 1);
            buildNoiseField_prgm->setUniform("bc_tx3D", 2);
            surface.resource->bindImage(2, GL_READ_ONLY);
            buildNoiseField_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));

            //buildNoiseField_prgm->dispatchCompute(static_cast<uint>(std::floor(brick.m_noise_params->getWidth() / 4)) + 1,
            //										static_cast<uint>(std::floor(brick.m_noise_params->getHeight() / 2)) + 1,
            //										static_cast<uint>(std::floor(brick.m_noise_params->getDepth() / 4)) + 1);

            int src = 0;

            // adapt iterations to ~1.5 times grid size
            if (iterations == 0)
                iterations = static_cast<int>(1.5f * std::max(
                        std::max(noise_params.resource->getWidth(), noise_params.resource->getHeight()),
                        noise_params.resource->getDepth())
                    );

            for (uint i = 0; i < iterations; i++)
            {
                noise_params.resource->bindImage(src, GL_READ_ONLY);
                backbuffer.bindImage((src + 1) % 2, GL_WRITE_ONLY);

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                glTextureBarrierNV();

                glDispatchCompute(static_cast<uint>(std::floor(noise_params.resource->getWidth() / 4)) + 1,
                    static_cast<uint>(std::floor(noise_params.resource->getHeight() / 2)) + 1,
                    static_cast<uint>(std::floor(noise_params.resource->getDepth() / 4)) + 1);

                src = (src == 0) ? 1 : 0;
            }

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Guidance noise field diffusion - " << iterations << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

        }

        void computeSurfacePropagation(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick,
            uint iterations = 0)
        {
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            auto surfacePropagationInit_prgm = resource_mngr.createShaderProgram("lcsp_surfacePropagationInit", { { "../resources/shaders/landscape/propagate_initBoundaryRegion_c.glsl", glowl::GLSLProgram::ShaderType::Compute} }).resource;
            auto surfacePropagation_prgm = resource_mngr.createShaderProgram("lcsp_surfacePropagation", { { "../resources/shaders/landscape/propagate_distApprox_c.glsl", glowl::GLSLProgram::ShaderType::Compute } }).resource;

            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto surface_boundaryRegion = resource_mngr.getTexture3DResource(brick.m_surface_boundaryRegion);
            auto eastern_boundary_3 = resource_mngr.getTexture3DResource(brick.m_eastern_boundary[3]);
            auto western_boundary_3 = resource_mngr.getTexture3DResource(brick.m_western_boundary[3]);
            auto upper_boundary_3 = resource_mngr.getTexture3DResource(brick.m_upper_boundary[3]);
            auto lower_boundary_3 = resource_mngr.getTexture3DResource(brick.m_lower_boundary[3]);
            auto northern_boundary_3 = resource_mngr.getTexture3DResource(brick.m_northern_boundary[3]);
            auto southern_boundary_3 = resource_mngr.getTexture3DResource(brick.m_southern_boundary[3]);

            auto eastern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_eastern_boundary[0]);
            auto western_boundary_0 = resource_mngr.getTexture3DResource(brick.m_western_boundary[0]);
            auto upper_boundary_0 = resource_mngr.getTexture3DResource(brick.m_upper_boundary[0]);
            auto lower_boundary_0 = resource_mngr.getTexture3DResource(brick.m_lower_boundary[0]);
            auto northern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_northern_boundary[0]);
            auto southern_boundary_0 = resource_mngr.getTexture3DResource(brick.m_southern_boundary[0]);

            auto normals = resource_mngr.getTexture3DResource(brick.m_normals);

            // TODO set textures of current brick to mirrored repeat

            // initialize boundary region
            surfacePropagationInit_prgm->use();

            // set boundary textures
            glActiveTexture(GL_TEXTURE3);
            eastern_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("east_tx3D", 3);
            glActiveTexture(GL_TEXTURE4);
            western_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("west_tx3D", 4);
            glActiveTexture(GL_TEXTURE5);
            upper_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("up_tx3D", 5);
            glActiveTexture(GL_TEXTURE6);
            lower_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("down_tx3D", 6);
            glActiveTexture(GL_TEXTURE7);
            northern_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("north_tx3D", 7);
            glActiveTexture(GL_TEXTURE8);
            southern_boundary_3.resource->bindTexture();
            surfacePropagationInit_prgm->setUniform("south_tx3D", 8);

            surfacePropagationInit_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            surfacePropagationInit_prgm->setUniform("cell_size", Vec3(brick.m_dimensions.x / brick.m_res_x,
                                                                    brick.m_dimensions.y / brick.m_res_y,
                                                                    brick.m_dimensions.z / brick.m_res_z));

            surfacePropagationInit_prgm->setUniform("surface_tx3D", 0);
            surfacePropagationInit_prgm->setUniform("boundaryRegion_tx3D", 1);
            surface.resource->bindImage(0, GL_READ_ONLY);
            surface_boundaryRegion.resource->bindImage(1, GL_WRITE_ONLY);

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

            glowl::TextureLayout r8ui_layout(GL_R8UI, brick.m_res_x, brick.m_res_y, brick.m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
                    std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

            // regionGrowing
            glowl::Texture3D boundaryRegion_backbuffer("boundaryRegion_backbuffer", r8ui_layout, nullptr);

            surfacePropagation_prgm->use();

            glActiveTexture(GL_TEXTURE0);
            surfacePropagation_prgm->setUniform("normals_tx3D", 0);
            normals.resource->bindTexture();

            surfacePropagation_prgm->setUniform("surface_tx3D", 0);
            surface.resource->bindImage(0, GL_READ_WRITE);

            // set boundary textures
            glActiveTexture(GL_TEXTURE3);
            eastern_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("east_tx3D", 3);
            glActiveTexture(GL_TEXTURE4);
            western_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("west_tx3D", 4);
            glActiveTexture(GL_TEXTURE5);
            upper_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("up_tx3D", 5);
            glActiveTexture(GL_TEXTURE6);
            lower_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("down_tx3D", 6);
            glActiveTexture(GL_TEXTURE7);
            northern_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("north_tx3D", 7);
            glActiveTexture(GL_TEXTURE8);
            southern_boundary_3.resource->bindTexture();
            surfacePropagation_prgm->setUniform("south_tx3D", 8);

            glActiveTexture(GL_TEXTURE9);
            eastern_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_east_tx3D", 9);
            glActiveTexture(GL_TEXTURE10);
            western_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_west_tx3D", 10);
            glActiveTexture(GL_TEXTURE11);
            upper_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_up_tx3D", 11);
            glActiveTexture(GL_TEXTURE12);
            lower_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_down_tx3D", 12);
            glActiveTexture(GL_TEXTURE13);
            northern_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_north_tx3D", 13);
            glActiveTexture(GL_TEXTURE14);
            southern_boundary_0.resource->bindTexture();
            surfacePropagation_prgm->setUniform("normals_south_tx3D", 14);

            surfacePropagation_prgm->setUniform("boundaryRegion_src_tx3D", 1);
            surfacePropagation_prgm->setUniform("boundaryRegion_tgt_tx3D", 2);

            surfacePropagation_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            surfacePropagation_prgm->setUniform("cell_size", Vec3(brick.m_dimensions.x / brick.m_res_x,
                brick.m_dimensions.y / brick.m_res_y,
                brick.m_dimensions.z / brick.m_res_z));

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // Adapt iterations to ~1.5 times grid size
            if (iterations == 0)
                iterations = static_cast<uint>(1.5f * std::max(
                    std::max(surface.resource->getWidth(), surface.resource->getHeight()),
                    surface.resource->getDepth())
                    );

            for (uint i = 0; i < iterations; i++)
            {
                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                surface_boundaryRegion.resource->bindImage(1, GL_READ_ONLY);
                boundaryRegion_backbuffer.bindImage(2, GL_WRITE_ONLY);

                glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);


                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                surface_boundaryRegion.resource->bindImage(2, GL_WRITE_ONLY);
                boundaryRegion_backbuffer.bindImage(1, GL_READ_ONLY);

                glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                    static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

            }

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Surface Propagation - " << iterations * 2 << " iterations - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

        }


        void smoothSurfaceField(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            // Smooth resulting surface field
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            auto smooth_prgm = resource_mngr.createShaderProgram("lcsp_smooth",{{ "../resources/shaders/seperatedGaussian3d_c.glsl", glowl::GLSLProgram::ShaderType::Compute} }).resource;
            auto copySurfaceField_prgm = resource_mngr.createShaderProgram("lcsp_copySurfaceField",{{ "../resources/shaders/copyTexture3D_R16_c.glsl", glowl::GLSLProgram::ShaderType::Compute} }).resource;
            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto surface_backbuffer = resource_mngr.getTexture3DResource(brick.m_surface_backbuffer);


            smooth_prgm->use();

            smooth_prgm->setUniform("stencilRadius", 2);

            smooth_prgm->setUniform("sigma", 0.8f);

            smooth_prgm->setUniform("src_tx3D", 0);
            //glBindImageTexture(0, brick.m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            surface.resource->bindImage(0, GL_READ_WRITE);
            smooth_prgm->setUniform("tgt_tx3D", 1);
            //glBindImageTexture(1, brick.m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            surface_backbuffer.resource->bindImage(1, GL_READ_WRITE);

            smooth_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));

            smooth_prgm->setUniform("offset", Vec3(1.0, 0.0, 0.0));

            glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


            //glBindImageTexture(1, brick.m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            //glBindImageTexture(0, brick.m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            surface.resource->bindImage(1, GL_READ_WRITE);
            surface_backbuffer.resource->bindImage(0, GL_READ_WRITE);

            smooth_prgm->setUniform("offset", Vec3(0.0, 1.0, 0.0));

            glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            //glBindImageTexture(0, brick.m_surface->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            //glBindImageTexture(1, brick.m_surface_backbuffer->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8);
            surface.resource->bindImage(0, GL_READ_WRITE);
            surface_backbuffer.resource->bindImage(1, GL_READ_WRITE);

            smooth_prgm->setUniform("offset", Vec3(0.0, 0.0, 1.0));

            glDispatchCompute(static_cast<uint>(std::floor(brick.m_res_x / 4)) + 1,
                static_cast<uint>(std::floor(brick.m_res_y / 2)) + 1,
                static_cast<uint>(std::floor(brick.m_res_z / 4)) + 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // Copy result to surface field
            copySurfaceField_prgm->use();
            copySurfaceField_prgm->setUniform("src_tx3D", 0);
            copySurfaceField_prgm->setUniform("tgt_tx3D", 1);
            copySurfaceField_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            surface_backbuffer.resource->bindImage(0, GL_READ_ONLY);
            surface.resource->bindImage(1, GL_WRITE_ONLY);
            glDispatchCompute(static_cast<uint>(std::floor(surface.resource->getWidth() / 4)) + 1,
                static_cast<uint>(std::floor(surface.resource->getHeight() / 2)) + 1,
                static_cast<uint>(std::floor(surface.resource->getDepth() / 4)) + 1);

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Smooth fields - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
        }


        void computeSurfaceMesh(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            std::cout << "Compute surface mesh" << std::endl;

            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            // See http://www.eriksmistad.no/marching-cubes-implementation-using-opencl-and-opengl/ for reference

            // Create texture for histogram pyramid (hp)
            int pyramid_lvls = static_cast<int>(std::log2(std::max(brick.m_res_x, std::max(brick.m_res_y, brick.m_res_z))) + 1);
            glowl::Texture3D** histogram_pyramid = new glowl::Texture3D * [pyramid_lvls];

            //TODO this get's generated a lot...
            auto generateTriangles_prgm = resource_mngr.createShaderProgram("lcsp_generateTriangles_L" + std::to_string(pyramid_lvls), { { "../resources/shaders/landscape/mc_generateTriangles_c.glsl", glowl::GLSLProgram::ShaderType::Compute} }, "#define HP_LVLS " + std::to_string(pyramid_lvls) + "\n").resource;
            auto classifyVoxels_prgm = resource_mngr.createShaderProgram("lcsp_classifyVoxels", { { "../resources/shaders/landscape/mc_classify_c.glsl", glowl::GLSLProgram::ShaderType::Compute } }).resource;

            // Generate prgms for different datatypes on different levels by insertig define files
            std::vector<Graphics::OpenGL::ResourceManager::ShaderFilename> const histo_shdr_src = { { "../resources/shaders/landscape/mc_buildHistogramPyramidLevel_c.glsl", glowl::GLSLProgram::ShaderType::Compute } };
            auto buildHpLvl_R8_prgm = resource_mngr.createShaderProgram("lcsp_buildHpLvl_R8", histo_shdr_src, "#define SRC_R8UI\n#define TGT_R8UI\n").resource;
            auto buildHpLvl_R8toR16_prgm = resource_mngr.createShaderProgram("lcsp_buildHpLvl_R8toR16", histo_shdr_src, "#define SRC_R8UI\n#define TGT_R16UI\n").resource;
            auto buildHpLvl_R16_prgm = resource_mngr.createShaderProgram("lcsp_buildHpLvl_R16", histo_shdr_src, "#define SRC_R16UI\n#define TGT_R16UI\n").resource;
            auto buildHpLvl_R16toR32_prgm = resource_mngr.createShaderProgram("lcsp_buildHpLvl_R16to32", histo_shdr_src, "#define SRC_R16UI\n#define TGT_R32UI\n").resource;
            auto buildHpLvl_R32_prgm = resource_mngr.createShaderProgram("lcsp_buildHpLvl_R32", histo_shdr_src, "#define SRC_R32UI\n#define TGT_R32UI\n").resource;

            auto surface = resource_mngr.getTexture3DResource(brick.m_surface);
            auto eastern_boundary_3 = resource_mngr.getTexture3DResource(brick.m_eastern_boundary[3]);
            auto upper_boundary_3 = resource_mngr.getTexture3DResource(brick.m_upper_boundary[3]);
            auto northern_boundary_3 = resource_mngr.getTexture3DResource(brick.m_northern_boundary[3]);

            //this->generateTriangles_prgms = new GLSLProgram * [5];
            //this->generateTriangles_prgms[0] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 6\n", "lcsp_generateTriangles_L6").resource;
            //this->generateTriangles_prgms[1] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 7\n", "lcsp_generateTriangles_L7").resource;
            //this->generateTriangles_prgms[2] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 8\n", "lcsp_generateTriangles_L8").resource;
            //this->generateTriangles_prgms[3] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 9\n", "lcsp_generateTriangles_L9").resource;
            //this->generateTriangles_prgms[4] = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/landscape/mc_generateTriangles_c.glsl" }, "#define HP_LVLS 10\n", "lcsp_generateTriangles_L10").resource;


            //GLSLProgram* generateTriangles_prgm;
            //
            //std::cout << "Pyramid levels: " << pyramid_lvls << std::endl;
            //
            //if (pyramid_lvls == 6)
            //    generateTriangles_prgm = generateTriangles_prgms[0];
            //else if (pyramid_lvls == 7)
            //    generateTriangles_prgm = generateTriangles_prgms[1];
            //else if (pyramid_lvls == 8)
            //    generateTriangles_prgm = generateTriangles_prgms[2];
            //else if (pyramid_lvls == 9)
            //    generateTriangles_prgm = generateTriangles_prgms[3];
            //else if (pyramid_lvls == 10)
            //    generateTriangles_prgm = generateTriangles_prgms[4];
            //else
            //    return;


            // Create texture for storing each cubes index (for marching cube lut)
            glowl::TextureLayout r8ui_layout(GL_R8UI, brick.m_res_x, brick.m_res_y, brick.m_res_z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1);
            glowl::Texture3D cube_indices("cube_indices", r8ui_layout, nullptr);

            for (int i = 0; i < pyramid_lvls; i++)
            {
                uint x = std::max(1, (int)((brick.m_res_x - 1) / pow(2, i)));
                uint y = std::max(1, (int)((brick.m_res_y - 1) / pow(2, i)));
                uint z = std::max(1, (int)((brick.m_res_z - 1) / pow(2, i)));

                glowl::TextureLayout r8ui_layout(GL_R8UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1);
                glowl::TextureLayout r16ui_layout(GL_R16UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 1);
                glowl::TextureLayout r32i_layout(GL_R32UI, x, y, z, GL_RED_INTEGER, GL_UNSIGNED_INT, 1);

                if (i <= 1)
                    histogram_pyramid[i] = new glowl::Texture3D("hp" + std::to_string(i), r8ui_layout, nullptr);
                else if (i > 1 && i < 5)
                    histogram_pyramid[i] = new glowl::Texture3D("hp" + std::to_string(i), r16ui_layout, nullptr);
                else
                    histogram_pyramid[i] = new glowl::Texture3D("hp" + std::to_string(i), r32i_layout, nullptr);
            }

            // Iso value for surface extraction
            float iso_value = 0.0;

            // Classify voxels
            classifyVoxels_prgm->use();
            classifyVoxels_prgm->setUniform("iso_value", iso_value);
            classifyVoxels_prgm->setUniform("surface_tx3D", 0);
            classifyVoxels_prgm->setUniform("histogram_pyramid_tx3D", 1);
            classifyVoxels_prgm->setUniform("cube_indices_tx3D", 2);
            surface.resource->bindImage(0, GL_READ_ONLY);
            histogram_pyramid[0]->bindImage(1, GL_WRITE_ONLY);
            cube_indices.bindImage(2, GL_WRITE_ONLY);

            glActiveTexture(GL_TEXTURE0);
            eastern_boundary_3.resource->bindTexture();
            classifyVoxels_prgm->setUniform("east_surface_tx3D", 0);
            glActiveTexture(GL_TEXTURE1);
            upper_boundary_3.resource->bindTexture();
            classifyVoxels_prgm->setUniform("up_surface_tx3D", 1);
            glActiveTexture(GL_TEXTURE2);
            northern_boundary_3.resource->bindTexture();
            classifyVoxels_prgm->setUniform("north_surface_tx3D", 2);

            glDispatchCompute(static_cast<uint>(std::floor(histogram_pyramid[0]->getWidth() / 4)),
                static_cast<uint>(std::floor(histogram_pyramid[0]->getHeight() / 2)),
                static_cast<uint>(std::floor(histogram_pyramid[0]->getDepth() / 4)));

            //std::cout<<"Pyramid levels: "<<pyramid_lvls<<std::endl;

            // Compute hp
            for (int i = 0; i < pyramid_lvls - 1; i++)
            {
                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                if (i == 0)
                {
                    buildHpLvl_R8_prgm->use();
                    buildHpLvl_R8_prgm->setUniform("src_tx3D", 0);
                    buildHpLvl_R8_prgm->setUniform("tgt_tx3D", 1);
                    histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
                    histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
                    glDispatchCompute(histogram_pyramid[i + 1]->getWidth(), histogram_pyramid[i + 1]->getHeight(), histogram_pyramid[i + 1]->getDepth());
                }
                else if (i == 1)
                {
                    buildHpLvl_R8toR16_prgm->use();
                    buildHpLvl_R8toR16_prgm->setUniform("src_tx3D", 0);
                    buildHpLvl_R8toR16_prgm->setUniform("tgt_tx3D", 1);
                    histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
                    histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
                    glDispatchCompute(histogram_pyramid[i + 1]->getWidth(), histogram_pyramid[i + 1]->getHeight(), histogram_pyramid[i + 1]->getDepth());
                }
                else if (i == 2 || i == 3)
                {
                    buildHpLvl_R16_prgm->use();
                    buildHpLvl_R16_prgm->setUniform("src_tx3D", 0);
                    buildHpLvl_R16_prgm->setUniform("tgt_tx3D", 1);
                    histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
                    histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
                    glDispatchCompute(histogram_pyramid[i + 1]->getWidth(), histogram_pyramid[i + 1]->getHeight(), histogram_pyramid[i + 1]->getDepth());
                }
                else if (i == 4)
                {
                    buildHpLvl_R16toR32_prgm->use();
                    buildHpLvl_R16toR32_prgm->setUniform("src_tx3D", 0);
                    buildHpLvl_R16toR32_prgm->setUniform("tgt_tx3D", 1);
                    histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
                    histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
                    glDispatchCompute(histogram_pyramid[i + 1]->getWidth(), histogram_pyramid[i + 1]->getHeight(), histogram_pyramid[i + 1]->getDepth());
                }
                else
                {
                    buildHpLvl_R32_prgm->use();
                    buildHpLvl_R32_prgm->setUniform("src_tx3D", 0);
                    buildHpLvl_R32_prgm->setUniform("tgt_tx3D", 1);
                    histogram_pyramid[i]->bindImage(0, GL_READ_WRITE);
                    histogram_pyramid[i + 1]->bindImage(1, GL_READ_WRITE);
                    glDispatchCompute(histogram_pyramid[i + 1]->getWidth(), histogram_pyramid[i + 1]->getHeight(), histogram_pyramid[i + 1]->getDepth());
                }
            }


            // Generate triangles (traverse hp)
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glUseProgram(0);
            GLuint triangle_count[1];
            histogram_pyramid[pyramid_lvls - 1]->bindTexture();
            glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, triangle_count);

            //std::cout<<"hp size: "<<histogram_pyramid[pyramid_lvls-1]->getWidth()*histogram_pyramid[pyramid_lvls-1]->getHeight()*histogram_pyramid[pyramid_lvls-1]->getDepth()<<std::endl;
            std::cout << "Triangle Count: " << triangle_count[0] << std::endl;

            // Adjust mesh size
            GLsizei vertices_byte_size = triangle_count[0] * 3 * 9 * 4; // triangle count * vertices per triangle * elements per vertex * byte per element
            GLsizei indices_byte_size = triangle_count[0] * 3 * 4; // triangle count * indices per triangle * byte size of index
            //brick.m_surface_mesh->rebuffer(nullptr, nullptr, vertices_byte_size, indices_byte_size, GL_PATCHES);
            brick.m_surface_mesh->rebufferVertexData(nullptr, vertices_byte_size);
            brick.m_surface_mesh->rebufferIndexData(nullptr, indices_byte_size);

            glowl::BufferObject triangle_table_ssbo(triangle_table);

            generateTriangles_prgm->use();

            generateTriangles_prgm->setUniform("iso_value", iso_value);
            generateTriangles_prgm->setUniform("grid_size", Vec3(brick.m_res_x, brick.m_res_y, brick.m_res_z));
            generateTriangles_prgm->setUniform("brick_size", brick.m_dimensions);

            glActiveTexture(GL_TEXTURE0);
            generateTriangles_prgm->setUniform("surface_tx3D", 0);
            surface.resource->bindTexture();

            glActiveTexture(GL_TEXTURE1);
            generateTriangles_prgm->setUniform("noise_tx3D", 1);
            brick.m_noise_params->bindTexture();

            glActiveTexture(GL_TEXTURE2);
            generateTriangles_prgm->setUniform("cube_indices_tx3D", 2);
            cube_indices.bindTexture();



            for (int i = 0; i < pyramid_lvls; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i + 6);
                std::string uniform_name("hp" + std::to_string(i) + "_tx3D");
                generateTriangles_prgm->setUniform(uniform_name.c_str(), i + 6);
                histogram_pyramid[i]->bindTexture();
            }

            triangle_table_ssbo.bind(0);

            uint tri_cnt = triangle_count[0];
            generateTriangles_prgm->setUniform("triangle_cnt", tri_cnt);

            // Bind vertex and index buffer as storage buffer
            brick.m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
            brick.m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 2);

            glDispatchCompute(static_cast<uint>(std::floor(tri_cnt / 32)) + 1, 1, 1);
            //generateTriangles_prgm->dispatchCompute(tri_cnt, 1, 1);

            // Add surface mesh to static meshes
            //GRenderingComponents::staticMeshManager().addComponent(brick.m_entity,brick.m_surface_mesh,brick.m_surface_material,false);

            glDispatchCompute(static_cast<uint>(std::floor(tri_cnt / 32)) + 1, 1, 1);


            // Clean up resources
            for (int i = 0; i < pyramid_lvls; i++)
                delete histogram_pyramid[i];

            delete[] histogram_pyramid;


            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until th e results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Surface Reconstruction - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
        }

        void computeNaiveSurfaceNetsMesh(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            std::unique_lock<std::mutex> ptex_lock(m_ptex_update);

            brick.m_cancel_ptex_update = true;

            std::cout << "Compute surface mesh" << std::endl;

            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            glowl::TextureLayout r32ui_layout(GL_R32UI,
                brick.m_res_x - 1,
                brick.m_res_y - 1,
                brick.m_res_z - 1,
                GL_RED_INTEGER,
                GL_UNSIGNED_INT,
                1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
                    std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

            glowl::TextureLayout r16i_layout(GL_R16UI,
                brick.m_res_x - 1,
                brick.m_res_y - 1,
                brick.m_res_z - 1,
                GL_RED_INTEGER,
                GL_UNSIGNED_SHORT,
                1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_REPEAT),
                std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_REPEAT),
                std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_REPEAT),
                std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

            glowl::Texture3D vertex_indices("surface_nets_vertex_indices", r32ui_layout, nullptr);

            glowl::Texture3D edge_crossings("surface_nets_edge_crossings", r16i_layout, nullptr);

            ShaderStorageBufferObject active_cubes((brick.m_res_x - 1) * (brick.m_res_y - 1) * (brick.m_res_z - 1) * 4 * 4, nullptr);
            std::vector<GLuint> data({ 0 });
            ShaderStorageBufferObject quad_counter_buffer(data);

            // Create atomic counter
            GLuint counter_buffer;
            glGenBuffers(1, &(counter_buffer));
            GLuint zero = 0;
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, counter_buffer);
            glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, 0);

            // Pass 1 - classify cubes

            // Iso value for surface extraction
            float iso_value = 0.0;

            // Classify voxels
            surfaceNets_classify_prgm->use();
            surfaceNets_classify_prgm->setUniform("iso_value", iso_value);
            surfaceNets_classify_prgm->setUniform("grid_size", Vec3(brick.m_res_x - 1, brick.m_res_y - 1, brick.m_res_z - 1));
            surfaceNets_classify_prgm->setUniform("brick_size", brick.m_dimensions);
            surfaceNets_classify_prgm->setUniform("surface_tx3D", 0);
            surfaceNets_classify_prgm->setUniform("vertex_indices_tx3D", 1);
            surfaceNets_classify_prgm->setUniform("edge_crossings_tx3D", 2);
            brick.m_surface->bindImage(0, GL_READ_ONLY);
            vertex_indices.bindImage(1, GL_WRITE_ONLY);
            edge_crossings.bindImage(2, GL_WRITE_ONLY);

            active_cubes.bind(0);
            quad_counter_buffer.bind(1);
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, counter_buffer);

            surfaceNets_classify_prgm->dispatchCompute(vertex_indices.getWidth() + 1,
                vertex_indices.getHeight() + 1,
                vertex_indices.getDepth() + 1);

            GLuint quad_cnt = 0;
            quad_counter_buffer.bind();
            GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            memcpy(&quad_cnt, p, 4);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

            std::cout << "Quad count: " << quad_cnt << std::endl;


            GLuint buffer_counter = 0;
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer);
            p = glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
            memcpy(&buffer_counter, p, 4);
            glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

            std::cout << "Active cubes: " << buffer_counter << std::endl;

            if (buffer_counter == 0)
                return;

            // Adjust mesh size
            GLsizei vertices_byte_size = buffer_counter * 9 * 4; // active cubes * elements per vertex * byte per element
            GLsizei indices_byte_size = quad_cnt * 4 * 4; // quad count * indices per quad * byte size of index
            //brick.m_surface_mesh->rebuffer(nullptr, nullptr, vertices_byte_size, indices_byte_size, GL_PATCHES);
            brick.m_surface_mesh->rebufferVertexData(nullptr, vertices_byte_size);
            brick.m_surface_mesh->rebufferIndexData(nullptr, indices_byte_size);

            //////////////////////////
            // PTEX BUFFERS
            //////////////////////////
            auto ptex_mesh_resource = resource_mngr.getMesh(brick.m_ptex_mesh);
            //ptex_mesh_resource.resource->rebuffer(nullptr, nullptr, buffer_counter * 6 * 4, quad_cnt * 4 * 4, GL_PATCHES);
            ptex_mesh_resource.resource->rebufferVertexData(nullptr, buffer_counter * 6 * 4);
            ptex_mesh_resource.resource->rebufferIndexData(nullptr, quad_cnt * 4 * 4);

            auto ptex_tiles_per_edge_resource = resource_mngr.getSSBO(brick.m_ptex_tiles_per_edge);
            std::vector<int> tiles_per_edges_init_buffer(buffer_counter * 6, -1);
            ptex_tiles_per_edge_resource.resource->reload(tiles_per_edges_init_buffer); // active cubes * 3 edges * 2 quad entries per edge * 4 byte per element

            auto ptex_parameters_resource = resource_mngr.getSSBO(brick.m_ptex_parameters);
            ptex_parameters_resource.resource->reload(quad_cnt * ((4 * 7)), 0, nullptr);
            auto ptex_parameters_backbuffer_resource = resource_mngr.getSSBO(brick.m_ptex_parameters_backbuffer);
            ptex_parameters_backbuffer_resource.resource->reload(quad_cnt * ((4 * 7)), 0, nullptr);

            auto patch_distances_SSBO_resource = resource_mngr.getSSBO(brick.m_ptex_patch_distances_SSBO);
            patch_distances_SSBO_resource.resource->reload(quad_cnt * 3 * 4, 0, nullptr);

            brick.m_ptex_patch_distances.resize(quad_cnt);
            //brick.m_ptex_patch_lod_classification.resize(quad_cnt);
            //////////////////////////
            // PTEX BUFFERS
            //////////////////////////

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // TODO Pass 2 - generate Quads
            surfaceNets_generateQuads_prgm->use();

            glActiveTexture(GL_TEXTURE0);
            surfaceNets_generateQuads_prgm->setUniform("surface_tx3D", 0);
            brick.m_surface->bindTexture();
            glActiveTexture(GL_TEXTURE1);
            surfaceNets_generateQuads_prgm->setUniform("noise_tx3D", 1);
            brick.m_noise_params->bindTexture();

            surfaceNets_generateQuads_prgm->setUniform("vertex_indices_tx3D", 1);
            surfaceNets_generateQuads_prgm->setUniform("edge_crossings_tx3D", 2);
            vertex_indices.bindImage(1, GL_READ_ONLY);
            edge_crossings.bindImage(2, GL_READ_ONLY);

            active_cubes.bind(0);
            // Bind vertex and index buffer as storage buffer
            brick.m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
            brick.m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 2);

            // Also fill ptex mesh and eventually ditch the old mesh
            ptex_mesh_resource.resource->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 3);
            ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 4);

            ptex_parameters_resource.resource->bind(5);
            ptex_tiles_per_edge_resource.resource->bind(6);

            surfaceNets_generateQuads_prgm->setUniform("iso_value", iso_value);
            surfaceNets_generateQuads_prgm->setUniform("grid_size", Vec3(brick.m_res_x - 1, brick.m_res_y - 1, brick.m_res_z - 1));
            surfaceNets_generateQuads_prgm->setUniform("brick_size", brick.m_dimensions);
            surfaceNets_generateQuads_prgm->setUniform("active_cubes_cnt", buffer_counter);

            surfaceNets_generateQuads_prgm->dispatchCompute(static_cast<uint>(floor(buffer_counter / 32) + 1), 1, 1);

            //std::vector<GLfloat> debug_vertices = {-10.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0, 0.0,
            //	10.0,0.0,10.0, 0.0, 0.0,1.0,0.0, 0.0, 1.0, 
            //	10.0,0.0,-10.0, 0.0, 0.0,1.0,0.0, 0.0, 1.0, 
            //	-8.0,0.0,-8.0, 0.0, 0.0,1.0,0.0, 0.0, 2.0 };
            //std::vector<GLuint> debug_indices = { 0,1,2,3 };
            //brick.m_surface_mesh->rebuffer(debug_vertices, debug_indices, GL_PATCHES);


            // Compute Ptex neighbourhood
            computePtexNeighbours_prgm->use();

            ptex_parameters_resource.resource->bind(0);

            //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ptex_mesh_resource.resource->getIboHandle());
            ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 4);

            ptex_tiles_per_edge_resource.resource->bind(6);

            computePtexNeighbours_prgm->setUniform("ptex_tile_cnt", quad_cnt);

            computePtexNeighbours_prgm->dispatchCompute(static_cast<uint>(std::floor(quad_cnt / 32) + 1), 1, 1);


            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            std::cout << "Surface Reconstruction - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

            auto gl_err = glGetError();
            if (gl_err != GL_NO_ERROR)
                std::cerr << "GL error in surface reconstruction: " << gl_err << std::endl;

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // Copy resulting quad surface mesh data to CPU memory
            size_t byte_size = ptex_mesh_resource.resource->getVbo().getByteSize();
            brick.m_mesh_vertex_data.resize(byte_size / 4);
            size_t element_cnt = brick.m_mesh_vertex_data.size();
            ptex_mesh_resource.resource->getVbo().bind();
            GLvoid* vertex_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, byte_size, GL_MAP_READ_BIT);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                // "Do something cop!"
                std::cerr << "GL error vertex data mapping: " << err << std::endl;
            }

            std::copy(reinterpret_cast<float*>(vertex_data), reinterpret_cast<float*>(vertex_data) + element_cnt, brick.m_mesh_vertex_data.data());
            glUnmapBuffer(GL_ARRAY_BUFFER);

            byte_size = ptex_mesh_resource.resource->getIbo().getByteSize();
            brick.m_mesh_index_data.resize(byte_size / 4);
            ptex_mesh_resource.resource->getIbo().bind();
            GLvoid* index_data = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
            memcpy(brick.m_mesh_index_data.data(), index_data, byte_size);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

            byte_size = ptex_parameters_resource.resource->getSize();
            brick.m_mesh_ptex_params.resize(quad_cnt); // 1 set of ptex params per quad
            ptex_parameters_resource.resource->bind();
            GLvoid* ptex_params = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
            memcpy(brick.m_mesh_ptex_params.data(), ptex_params, byte_size);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


            // Bake intial surface textures
            bakeSurfaceTextures(index);
        }

        void bakeSurfaceTextures(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            std::cout << "Texture Baking" << std::endl;

            uint primitive_cnt = brick.m_surface_mesh->getIndicesCount() / 4; //Quad primitives -> 4 indices per primitive

            std::cout << "Primitive count (texture baking stage): " << primitive_cnt << std::endl;

            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            // rebuffer ptex mesh, textures and parameters
            WeakResource<Mesh>						ptex_mesh_resource = resource_mngr.getMesh(brick.m_ptex_mesh);
            WeakResource<ShaderStorageBufferObject>	ptex_bindless_texture_handles_resource = resource_mngr.getSSBO(brick.m_ptex_bindless_texture_handles);
            WeakResource<ShaderStorageBufferObject>	ptex_bindless_images_handles_resource = resource_mngr.getSSBO(brick.m_ptex_bindless_image_handles);
            WeakResource<ShaderStorageBufferObject>	ptex_bindless_mipmap_image_handles_resource = resource_mngr.getSSBO(brick.m_ptex_bindless_mipmap_image_handles);
            WeakResource<ShaderStorageBufferObject>	ptex_parameters_resource = resource_mngr.getSSBO(brick.m_ptex_parameters);

            int material_components = 4;
            GLint layers;
            glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &layers);
            layers = layers - (layers % material_components);

            // TODO query GPU vendor and subsequently the available video memory to make an assumption about how much memory I want to spend on Ptex textures
            size_t available_ptex_memory = 1500000000; // GPU memory available for ptex textures given in byte
            size_t used_ptex_memory = 0;
            size_t texture_array_cnt = 0;

            // create vista texture tiles (8x8) for all surface patches
            size_t vista_tiles_array_cnt = (1 + ((primitive_cnt * material_components + 1) / layers));
            size_t vista_tiles_memory = vista_tiles_array_cnt * layers * (8 * 8) * 4;
            used_ptex_memory += vista_tiles_memory;
            texture_array_cnt += vista_tiles_array_cnt;

            // if more than ~1GB of available memory left, use 256x256 detail tiles
            brick.m_lod_lvls = ((available_ptex_memory - used_ptex_memory) > 1000000000) ? 6 : 5;
            //brick.m_lod_lvls = ((available_ptex_memory - used_ptex_memory) > 1000000000) ? 7 : 6;
            brick.m_ptex_lod_bin_sizes.resize(brick.m_lod_lvls);

            brick.m_ptex_active_patch_lod_classification.clear();
            brick.m_ptex_active_patch_lod_classification.resize(primitive_cnt, brick.m_lod_lvls - 1); // intialize with max lod level (i.e. vista layer)

            brick.m_ptex_latest_patch_lod_classification.clear();
            brick.m_ptex_latest_patch_lod_classification.resize(primitive_cnt, brick.m_lod_lvls - 1); // intialize with max lod level (i.e. vista layer)

            // use one set of detail tiles
            size_t detail_tiles_memory = layers * static_cast<size_t>(std::pow(2, brick.m_lod_lvls + 2)) * static_cast<size_t>(std::pow(2, brick.m_lod_lvls + 2)) * 4;
            used_ptex_memory += detail_tiles_memory;
            texture_array_cnt += 1;
            brick.m_ptex_lod_bin_sizes[0] = layers / material_components;

            assert(available_ptex_memory > used_ptex_memory);

            // split remaining memory evenly among remaining lod levels
            size_t memory_per_lvl = static_cast<size_t>(std::floor(((available_ptex_memory - used_ptex_memory) / (brick.m_lod_lvls - 2))));

            for (int i = 1; i < brick.m_lod_lvls - 1; ++i) // skip detail and vista level
            {
                // compute number of texture arrays per level based on available memory per level
                size_t array_memory = layers * (std::pow(2, (brick.m_lod_lvls + 2) - i) * std::pow(2, (brick.m_lod_lvls + 2) - i)) * 4;
                size_t array_cnt = static_cast<size_t>(std::floor(memory_per_lvl / array_memory));
                brick.m_ptex_lod_bin_sizes[i] = (array_cnt * layers) / material_components;
                used_ptex_memory += array_memory * array_cnt;
                texture_array_cnt += array_cnt;
            }

            brick.m_ptex_lod_bin_sizes[brick.m_lod_lvls - 1] = (vista_tiles_array_cnt * layers) / material_components;

            // clear current update target tiles (basically cancel current update) as it will no longer match the surface mesh
            brick.m_ptex_updatePatches_tgt.clear();

            // intialize available tiles (at the beginning, all tiles on all levels are available). exclude vista layer
            brick.m_ptex_active_availableTiles.resize(brick.m_lod_lvls - 1);
            brick.m_ptex_latest_availableTiles.resize(brick.m_lod_lvls - 1);
            uint32_t assigned_tiles_a = 0;
            uint32_t assigned_tiles_l = 0;
            for (int i = 0; i < brick.m_lod_lvls - 1; ++i)
            {
                brick.m_ptex_active_availableTiles[i].resize(brick.m_ptex_lod_bin_sizes[i]);
                brick.m_ptex_latest_availableTiles[i].resize(brick.m_ptex_lod_bin_sizes[i]);
                //std::iota(brick.m_ptex_availableTiles[i].begin(), brick.m_ptex_availableTiles[i].end(), 0);
                for (auto& slot : brick.m_ptex_active_availableTiles[i])
                {
                    LandscapeBrickComponent::TextureSlot new_slot;
                    new_slot.tex_index = (assigned_tiles_a / layers);
                    new_slot.base_slice = assigned_tiles_a % 2048;
                    slot = new_slot;

                    assigned_tiles_a += material_components;
                }

                for (auto& slot : brick.m_ptex_latest_availableTiles[i])
                {
                    LandscapeBrickComponent::TextureSlot new_slot;
                    new_slot.tex_index = (assigned_tiles_l / layers);
                    new_slot.base_slice = assigned_tiles_l % 2048;
                    slot = new_slot;

                    assigned_tiles_l += material_components;
                }
            }

            //DEBUGGING
            //for (auto v : brick.m_ptex_availableTiles[0])
            //{
            //	std::cout << "Available tile:" << v.tex_index << " " << v.base_slice << std::endl;
            //}

            std::cout << "Num texture arrays: " << texture_array_cnt << std::endl;
            std::cout << "Num layers per array: " << layers << std::endl;
            std::cout << "Num lod levels: " << brick.m_lod_lvls << std::endl;

            std::vector<GLuint64> texture_handles;
            texture_handles.reserve(texture_array_cnt);
            std::vector<GLuint64> image_handles;
            std::vector<GLuint64> mipmap_image_handles;
            image_handles.reserve(texture_array_cnt);
            brick.m_ptex_textures.clear();
            brick.m_ptex_textures.reserve(texture_array_cnt);

            //std::vector<std::vector<uint8_t>> debug_image_data(lod_lvls);

            for (size_t i = 0; i < brick.m_lod_lvls; ++i)
            {
                glowl::TextureLayout tile_layout;
                tile_layout.internal_format = GL_RGBA8;
                tile_layout.format = GL_RGBA;
                tile_layout.type = GL_UNSIGNED_BYTE;
                tile_layout.width = static_cast<int>(std::pow(2, (brick.m_lod_lvls + 2) - i)); //start at 8
                tile_layout.height = static_cast<int>(std::pow(2, (brick.m_lod_lvls + 2) - i)); // times 8
                tile_layout.depth = layers;
                tile_layout.levels = 2;

                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
                //tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f });
                tile_layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER, GL_LINEAR });

                //debug_image_data[i] = std::vector<uint8_t>(tile_layout.width*tile_layout.height*tile_layout.depth*4,255);

                size_t array_cnt = (brick.m_ptex_lod_bin_sizes[i] * material_components) / layers;
                for (size_t j = 0; j < array_cnt; ++j)
                {
                    auto tx_resource = resource_mngr.createTexture2DArray("brick_" + std::to_string(index) + "_ptex_lvl_" + std::to_string(i) + "_array_ " + std::to_string(j), tile_layout, nullptr, false);
                    //auto tx_resource = resource_mngr.createTexture2DArray("brick_" + std::to_string(index) + "_ptex_lvl_" + std::to_string(i) + "_array_ " + std::to_string(j), tile_layout, debug_image_data[i].data(), false);
                    brick.m_ptex_textures.push_back(tx_resource.id);

                    GLenum err = glGetError();
                    if (err != GL_NO_ERROR)
                    {
                        // "Do something cop!"
                        //std::cerr << "GL error during texture baking creation after texture creation: " << err << std::endl;
                    }

                    texture_handles.push_back(tx_resource.resource->getTextureHandle());
                    image_handles.push_back(tx_resource.resource->getImageHandle(0, GL_TRUE, 0));
                    mipmap_image_handles.push_back(tx_resource.resource->getImageHandle(1, GL_TRUE, 0)); // we use two mipmap levels for ptex tiles so query both levels

                    tx_resource.resource->makeResident();
                    glMakeImageHandleResidentARB(image_handles.back(), GL_WRITE_ONLY);
                    glMakeImageHandleResidentARB(mipmap_image_handles.back(), GL_WRITE_ONLY);
                }
            }

            /*
            std::vector<uint8_t> image_data;
            glowl::TextureLayout image_layout;
            //ResourceLoading::loadPpmImageRGBA("../resources/textures/debug_uv_tile_64x64.ppm", image_data, image_layout);

            image_layout.internal_format = GL_RGBA8;
            image_layout.format = GL_RGBA;
            image_layout.type = GL_UNSIGNED_BYTE;
            image_layout.width = 16;
            image_layout.height = 16;
            image_layout.depth = layers;

            //std::vector<uint8_t> array_dummy_data(image_data.size()*layers, 255);
            //for (int i = 0; i < layers; i++)
            //{
            //	std::copy(image_data.data(), image_data.data() + image_data.size(), array_dummy_data.data() + (i * image_data.size()));
            //}

            for (uint i = 0; i < num_tx_arrays; i++)
            {
                glowl::TextureLayout layout = image_layout;

                layout.depth = layers;
                layout.int_parameters.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER });
                layout.int_parameters.push_back({ GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER });
                layout.int_parameters.push_back({ GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER });
                layout.int_parameters.push_back({ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR });
                layout.int_parameters.push_back({ GL_TEXTURE_MAG_FILTER,GL_LINEAR });
                //layout.int_parameters.push_back({ GL_TEXTURE_SPARSE_ARB,GL_TRUE });

                //auto tx_resource = resource_mngr.createTexture2DArray("brick_" + std::to_string(index) + "_ptex_texture_" + std::to_string(i), layout, array_dummy_data.data(), false);
                auto tx_resource = resource_mngr.createTexture2DArray("brick_" + std::to_string(index) + "_ptex_texture_" + std::to_string(i), layout, nullptr, false);
                brick.m_ptex_textures.push_back(tx_resource.entity);

                GLenum err = glGetError();
                if (err != GL_NO_ERROR)
                {
                    // "Do something cop!"
                    std::cerr << "GL error during texture baking creation after texture creation: " << err << std::endl;
                }

                texture_handles[i] = tx_resource.resource->getTextureHandle();
                image_handles[i] = tx_resource.resource->getImageHandle(0, GL_TRUE, 0);

                tx_resource.resource->makeResident();
                glMakeImageHandleResidentARB(image_handles[i], GL_WRITE_ONLY);
            }

            */

            glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
            ptex_bindless_texture_handles_resource.resource->reload(texture_handles);
            ptex_bindless_images_handles_resource.resource->reload(image_handles);
            ptex_bindless_mipmap_image_handles_resource.resource->reload(mipmap_image_handles);

            // load bindless texture handles for all texture given by ptex material to make them available during texture baking
            std::vector<GLuint64> surface_texture_handles;
            WeakResource<Material> ptex_material_resource = resource_mngr.getMaterial(brick.m_ptex_material);
            for (auto texture : ptex_material_resource.resource->getTextures())
            {
                surface_texture_handles.push_back(texture->getTextureHandle());
                texture->makeResident();
            }
            WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = resource_mngr.getSSBO(brick.m_ptex_material_bth);
            ptex_material_bth_resource.resource->reload(surface_texture_handles);

            {
                auto err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cerr << "Error - bakeSurfaceTexture - 3953: " << err << std::endl;
                }
            }

            // Bake surface textures
            textureBaking_prgm->use();

            // Bind vertex and index buffer as storage buffer
            brick.m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
            brick.m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

            ptex_bindless_images_handles_resource.resource->bind(2);
            ptex_parameters_resource.resource->bind(3);

            ptex_material_bth_resource.resource->bind(4);

            ResourceID decal_buffer = GRenderingComponents::decalManager().getGPUBufferResource();
            auto decal_buffer_rsrc = resource_mngr.getSSBO(decal_buffer);
            decal_buffer_rsrc.resource->bind(5);

            textureBaking_prgm->setUniform("decal_cnt", GRenderingComponents::decalManager().getComponentCount());
            textureBaking_prgm->setUniform("texture_lod", static_cast<float>(brick.m_lod_lvls));
            textureBaking_prgm->setUniform("layers", layers);

            int texture_base_idx = image_handles.size() - ((brick.m_ptex_lod_bin_sizes[brick.m_lod_lvls - 1] * material_components) / layers);
            textureBaking_prgm->setUniform("texture_base_idx", texture_base_idx);

            GLint primitive_base_idx = 0;
            GLint remaining_dispatches = primitive_cnt;
            GLint max_work_groups_z = 0;
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &max_work_groups_z);

            while (remaining_dispatches > 0)
            {
                GLuint dispatchs_cnt = (remaining_dispatches > max_work_groups_z) ? max_work_groups_z : remaining_dispatches;

                textureBaking_prgm->setUniform("primitive_base_idx", primitive_base_idx);
                textureBaking_prgm->dispatchCompute(1, 1, dispatchs_cnt);

                if (remaining_dispatches > max_work_groups_z)
                {
                    remaining_dispatches -= max_work_groups_z;
                    primitive_base_idx += max_work_groups_z;
                }
                else
                {
                    remaining_dispatches = 0;
                }
            }

            // Build vista tiles mipmaps
            for (int i = texture_base_idx; i < brick.m_ptex_textures.size(); ++i)
            {
                auto ptex_texture_resource = resource_mngr.getTexture2DArray(brick.m_ptex_textures[i]);

                ptex_texture_resource.resource->bindTexture();
                glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            }

            //(Re)build mipmaps...seems slow
            //for (auto ptex_texture : brick.m_ptex_textures)
            //{
            //	auto ptex_texture_resource = resource_mngr.getTexture2DArray(ptex_texture);
            //
            //	ptex_texture_resource.resource->bindTexture();
            //	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            //}


            // Recompute ptex update buffers to overwrite any computations done asynchrously by tasks spawned before the terrain update was called
            computePatchDistances(index);
            computeTextureTileUpdateList(index);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                // "Do something cop!"
                std::cerr << "GL error during texture baking." << err << std::endl;
            }

            // bake lod texture tiles (intial lod baking differs from updating tiles..)
            //brick.m_ptex_ready = true;
            //updateTextureBaking(index);

            //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
            //{
            //	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //
            //computePatchDistances(index);
            //
            //GEngineCore::taskSchedueler().submitTask([this, index]() {
            //
            //	std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //
            //	computeTextureTileUpdateList(index);
            //
            //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
            //	{
            //		std::unique_lock<std::mutex> ptex_lock(m_ptex_update);
            //		updateTextureTiles(index);
            //	});
            //
            //});
            //});

            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            brick.m_ptex_ready = true;

            std::cout << "Texture Baking - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;

        }

        void computePatchDistances(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            auto start_time = std::chrono::steady_clock::now();

            size_t quad_cnt = brick.m_mesh_ptex_params.size();

            brick.m_ptex_patch_distances.resize(quad_cnt);

            // get camera transform
            Mat4x4 model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(brick.m_entity));
            Mat4x4 view_mx = GEngineCore::frameManager().getRenderFrame().m_view_matrix;
            Mat4x4 proj_mx = GEngineCore::frameManager().getRenderFrame().m_projection_matrix;

            for (size_t quad_idx = 0; quad_idx < quad_cnt; ++quad_idx)
            {
                uint32_t patch_indices[4];
                Vec4 patch_vertices[4];

                Vec3 midpoint(0.0f);

                for (int i = 0; i < 4; ++i)
                {
                    patch_indices[i] = brick.m_mesh_index_data[quad_idx * 4 + i];

                    patch_vertices[i].x = brick.m_mesh_vertex_data[patch_indices[i] * 6 + 0];
                    patch_vertices[i].y = brick.m_mesh_vertex_data[patch_indices[i] * 6 + 1];
                    patch_vertices[i].z = brick.m_mesh_vertex_data[patch_indices[i] * 6 + 2];
                    patch_vertices[i].w = 1.0;

                    //patch_vertices[i] = model_view_mx * patch_vertices[i];

                    midpoint += Vec3(patch_vertices[i]);
                }

                midpoint = midpoint / 4.0f;

                LandscapeBrickComponent::PatchInfo patch_info;
                //distances[gID_x] = length(camera_position - midpoint);
                //distances[gID_x] = length((inverse(view_mx)*vec4(0.0,0.0,0.0,1.0)).xyz - midpoint);
                //patch_info.distance = glm::length(Vec3(view_mx * Vec4(midpoint, 1.0))); //TODO check model matrix
                Vec3 camera_position = Vec3(glm::inverse(view_mx) * Vec4(0.0, 0.0, 0.0, 1.0));
                camera_position.x = std::floor(camera_position.x / 4.0f) * 4.0f;
                camera_position.y = std::floor(camera_position.y / 4.0f) * 4.0f;
                camera_position.z = std::floor(camera_position.z / 4.0f) * 4.0f;
                patch_info.midpoint = midpoint;
                patch_info.distance = glm::length(camera_position - midpoint); //TODO check model matrix
                patch_info.tex_index = brick.m_mesh_ptex_params[quad_idx].texture_index;
                patch_info.base_slice = brick.m_mesh_ptex_params[quad_idx].base_slice;

                // "Frustrum" culling
                bool frustrum_test[4] = { false,false,false,false };

                for (int i = 0; i < 4; ++i)
                {
                    // Transform patch vertices to clip space
                    //vec4 cs_pos = proj_mx * view_mx * model_mx * patch_vertices[i];

                    Vec4 cs_pos = proj_mx * view_mx * patch_vertices[i]; //TODO check model matrix

                    cs_pos.w = cs_pos.w * 1.25;

                    // Test against frustrum, if outside set very large distance
                    if (cs_pos.x < -cs_pos.w || cs_pos.x > cs_pos.w)
                        frustrum_test[i] = true;

                    if (cs_pos.y < -cs_pos.w || cs_pos.y > cs_pos.w)
                        frustrum_test[i] = true;

                    if (cs_pos.z < -cs_pos.w || cs_pos.z > cs_pos.w)
                        frustrum_test[i] = true;
                }

                if (frustrum_test[0] && frustrum_test[1] && frustrum_test[2] && frustrum_test[3])
                    patch_info.distance = 9999.0;


                brick.m_ptex_patch_distances[quad_idx] = patch_info;
            }

            auto t_1 = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> time = (t_1 - start_time);
            brick.ptex_distance_computation_time += (time.count()) / 1000000.0;
            //std::cout << "Distance computation - " << (time.count()) / 1000000.0 << "ms" << std::endl;


            // GLuint64 t_0, t_1;
            // unsigned int queryID[2];
            // // generate two queries
            // glGenQueries(2, queryID);
            // glQueryCounter(queryID[0], GL_TIMESTAMP);
            // 
            // glMemoryBarrier(GL_ALL_BARRIER_BITS);
            // 
            // this->computePatchDistances_prgm->use();
            // 
            // // bind ptex vertex and index buffer
            // auto ptex_mesh_resource = resource_mngr.getMesh(brick.m_ptex_mesh);
            // ptex_mesh_resource.resource->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
            // ptex_mesh_resource.resource->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);
            // 
            // 
            // // bind distance values buffer
            // auto patch_distances_SSBO_resource = resource_mngr.getSSBO(brick.m_ptex_patch_distances_SSBO);
            // if (patch_distances_SSBO_resource.state != READY)
            // 	return;
            // 
            // patch_distances_SSBO_resource.resource->bind(2);
            // 
            // auto ptex_params_resource = resource_mngr.getSSBO(brick.m_ptex_parameters);
            // ptex_params_resource.resource->bind(3);
            // 
            // // set camera position uniform
            // Mat4x4 model_mx = GCoreComponents::transformManager().getWorldTransformation(GCoreComponents::transformManager().getIndex(brick.m_entity));
            // Mat4x4 view_mx = GEngineCore::frameManager().getRenderFrame().m_view_matrix;
            // Mat4x4 proj_mx = GEngineCore::frameManager().getRenderFrame().m_projection_matrix;
            // //Mat4x4 model_view_mx = view_mx *model_mx;
            // auto camera_postion = glm::inverse(view_mx) * Vec4(0.0,0.0,0.0,1.0);
            // Vec3 camera_postion_uniform_value = Vec3(camera_postion.x, camera_postion.y, camera_postion.z);
            // computePatchDistances_prgm->setUniform("camera_position", camera_postion_uniform_value);
            // computePatchDistances_prgm->setUniform("model_mx", view_mx);
            // computePatchDistances_prgm->setUniform("view_mx", view_mx);
            // computePatchDistances_prgm->setUniform("proj_mx", proj_mx);
            // 
            // 
            // glMemoryBarrier(GL_ALL_BARRIER_BITS);
            // 
            // // dispatch compute
            // uint patch_cnt = ptex_mesh_resource.resource->getIndicesCount() / 4; //Quad primitives -> 4 indices per primitive
            // computePatchDistances_prgm->setUniform("ptex_patch_cnt", patch_cnt);
            // computePatchDistances_prgm->dispatchCompute( static_cast<uint>(std::floor(patch_cnt/32)+1),1,1);
            // 
            // // map distance values buffer to CPU memory
            // brick.m_ptex_patch_distances.resize(patch_cnt);
            // patch_distances_SSBO_resource.resource->bind();
            // GLvoid* distance_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            // memcpy(brick.m_ptex_patch_distances.data(), distance_data, 4/*byte*/ * 3/*elements*/ * patch_cnt);
            // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            // 
            // 
            // glQueryCounter(queryID[1], GL_TIMESTAMP);
            // 
            // // wait until the results are available
            // GLint stopTimerAvailable = 0;
            // while (!stopTimerAvailable)
            // 	glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
            // 
            // // get query results
            // glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            // glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);
            // 
            // //std::cout << "Distance computation - " << (t_1 - t_0) / 1000000.0 << "ms" << std::endl;
            // brick.ptex_distance_computation_time += (t_1 - t_0) / 1000000.0;

            // DEBUGGING
            //	for (auto v : brick.m_ptex_patch_distances)
            //	{
            //		std::cout << v.distance << std::endl;
            //	}
        }

        void computeTextureTileUpdateList(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            auto t_0 = std::chrono::steady_clock::now();

            // sort distance values, keep track of original indices
            size_t patch_cnt = brick.m_ptex_patch_distances.size();
            int max_lod_lvl = brick.m_ptex_lod_bin_sizes.size() - 1;

            // initialize original index locations
            std::vector<size_t> patch_indices(patch_cnt);
            std::iota(patch_indices.begin(), patch_indices.end(), 0);

            // sort indexes based on comparing patch distance values
            std::sort(patch_indices.begin(), patch_indices.end(),
                [this, index](size_t i1, size_t i2) {return brick.m_ptex_patch_distances[i1].distance < brick.m_ptex_patch_distances[i2].distance; });

            // DEBUGING
            //for (int i=0; i< patch_indices.size(); ++i)
            //{
            //	std::cout << brick.m_ptex_patch_distances[patch_indices[i]] << " : " << patch_indices[i] << std::endl;
            //}

            // classify distance values based on LOD bin sizes
            std::vector<int> classification(patch_cnt);
            size_t remaining_patches = patch_cnt;

            brick.m_ptex_latest_availableTiles = brick.m_ptex_active_availableTiles;

            //std::vector<float> lod_distance_steps({ 11.5f,23.0f,45.0f,88.0f,175.0f,999999.0f }); // values measured for optimal mipmap level
            std::vector<float> lod_distance_steps({ 12.0f,24.0f,48.0f,96.0f,192.0f,999999.0f }); // values measured for optimal mipmap level
            std::vector<uint> remaining_lod_bin_size = brick.m_ptex_lod_bin_sizes;

            int lod_bin = 0;
            for (auto patch_idx : patch_indices)
            {
                while ((remaining_lod_bin_size[lod_bin] == 0) || (brick.m_ptex_patch_distances[patch_idx].distance >= lod_distance_steps[lod_bin]))
                {
                    if (lod_bin == max_lod_lvl)
                        break;

                    ++lod_bin;
                }

                classification[patch_idx] = lod_bin;

                remaining_lod_bin_size[lod_bin] = remaining_lod_bin_size[lod_bin] - 1;
            }

            // compare classification with previous classification
            size_t update_patches_cnt = 0;
            std::vector<std::vector<uint>> update_patches_tgt(brick.m_ptex_lod_bin_sizes.size());

            // Vista LoD doesnt need to be included for freed and available slots
            std::vector<std::list<LandscapeBrickComponent::TextureSlot>> freed_slots(brick.m_ptex_active_availableTiles.size());

            for (uint i = 0; i < classification.size(); ++i)
            {
                int previous_classification = brick.m_ptex_active_patch_lod_classification[i];

                if (classification[i] != previous_classification)
                {
                    size_t patch_idx = i;

                    update_patches_tgt[classification[i]].push_back(patch_idx); // add patch to update list with new classification

                    if (previous_classification != max_lod_lvl) // if previously not vista LOD add to available texture slots
                    {
                        LandscapeBrickComponent::TextureSlot free_slot;
                        free_slot.tex_index = brick.m_ptex_patch_distances[i].tex_index;
                        free_slot.base_slice = brick.m_ptex_patch_distances[i].base_slice;
                        //brick.m_ptex_latest_availableTiles[previous_classification].push_back(free_slot);
                        freed_slots[previous_classification].push_back(free_slot);

                        //TODO assert tex index fits bin index
                    }

                    brick.m_ptex_latest_patch_lod_classification[i] = classification[i]; // update stored classification

                    ++update_patches_cnt;
                }
            }

            // Sort update patches based on ?? to encourgage clustered assignment of texture index and base slices
            //	for (int i = 0; i < update_patches_tgt.size(); ++i)
            //	{
            //		std::sort(update_patches_tgt[i].begin(), update_patches_tgt[i].end(),[this,index](const uint & a, const uint & b) -> bool
            //		{
            //			float distance_A = std::floor(brick.m_ptex_patch_distances[a].midpoint.x / 4.0f) * 400.0
            //				+ std::floor(brick.m_ptex_patch_distances[a].midpoint.y / 4.0f) *40.0f
            //				+ std::floor(brick.m_ptex_patch_distances[a].midpoint.z / 4.0f) *4.0f;
            //			float distance_B = std::floor(brick.m_ptex_patch_distances[b].midpoint.x / 4.0f) * 400.0
            //				+ std::floor(brick.m_ptex_patch_distances[b].midpoint.y / 4.0f) *40.0f
            //				+ std::floor(brick.m_ptex_patch_distances[b].midpoint.z / 4.0f) *4.0f;
            //	
            //			return distance_A > distance_B;
            //	
            //			//uint32_t tex_index_A = brick.m_ptex_patch_distances[a].tex_index;
            //			//uint32_t tex_index_B = brick.m_ptex_patch_distances[b].tex_index;
            //			//uint32_t base_slice_A = brick.m_ptex_patch_distances[a].base_slice;
            //			//uint32_t base_slice_B = brick.m_ptex_patch_distances[a].base_slice;
            //	
            //			//return (tex_index_A != tex_index_B) ? (tex_index_A > tex_index_B) : (base_slice_A > base_slice_B);
            //		});
            //	}

            // copy update patches to continous memory
            brick.m_ptex_updatePatches_tgt.clear();
            brick.m_ptex_updatePatches_tgt.resize(update_patches_cnt);

            brick.m_ptex_update_bin_sizes.clear();
            //brick.m_ptex_update_bin_sizes.reserve(brick.m_ptex_lod_bin_sizes.size());

            std::ostringstream bin_size_log;

            size_t tgt_copied_elements = 0;
            for (size_t i = 0; i < brick.m_ptex_lod_bin_sizes.size(); ++i)
            {
                std::copy(update_patches_tgt[i].begin(), update_patches_tgt[i].end(), brick.m_ptex_updatePatches_tgt.begin() + tgt_copied_elements);

                tgt_copied_elements += update_patches_tgt[i].size();

                bin_size_log << "Update bin size:" << update_patches_tgt[i].size() << std::endl;

                brick.m_ptex_update_bin_sizes.push_back(update_patches_tgt[i].size());
            }

            // Sort both available and newly freed texture tile slots independent apped freed slots to available slot lists afterwards
            for (int i = 0; i < freed_slots.size(); ++i)
            {
                brick.m_ptex_latest_availableTiles[i].sort([](const LandscapeBrickComponent::TextureSlot& a, const LandscapeBrickComponent::TextureSlot& b) -> bool
                    {
                        return (a.tex_index != b.tex_index) ? (a.tex_index > b.tex_index) : (a.base_slice > b.base_slice);
                    });

                freed_slots[i].sort([](const LandscapeBrickComponent::TextureSlot& a, const LandscapeBrickComponent::TextureSlot& b) -> bool
                    {
                        return (a.tex_index != b.tex_index) ? (a.tex_index > b.tex_index) : (a.base_slice > b.base_slice);
                    });

                brick.m_ptex_latest_availableTiles[i].splice(brick.m_ptex_latest_availableTiles[i].end(), freed_slots[i]);
            }

            size_t available_tiles_cnt = 0;
            for (auto& tile_list : brick.m_ptex_latest_availableTiles)
                available_tiles_cnt += tile_list.size();

            brick.m_ptex_availableTiles_uploadBuffer.clear();
            brick.m_ptex_availableTiles_uploadBuffer.resize(available_tiles_cnt);
            brick.m_ptex_availableTiles_bin_sizes.clear();

            std::ostringstream available_tiles_log;

            size_t copied_elements = 0;
            for (size_t i = 0; i < brick.m_ptex_latest_availableTiles.size(); ++i)
            {
                bin_size_log << "Available tiles:" << brick.m_ptex_latest_availableTiles[i].size() << std::endl;

                std::copy(brick.m_ptex_latest_availableTiles[i].begin(), brick.m_ptex_latest_availableTiles[i].end(), brick.m_ptex_availableTiles_uploadBuffer.begin() + copied_elements);

                copied_elements += brick.m_ptex_latest_availableTiles[i].size();

                //available_tiles_log << "Available tiles:" << brick.m_ptex_availableTiles[i].size() << std::endl;

                brick.m_ptex_availableTiles_bin_sizes.push_back(brick.m_ptex_latest_availableTiles[i].size());
            }

            if (tgt_copied_elements > 0)
                std::cout << bin_size_log.str() << available_tiles_log.str();

            // After available texture tiles are copied to upload buffer, remove as many as will be used by update patches
            for (size_t i = 0; i < brick.m_ptex_latest_availableTiles.size(); ++i)
            {
                auto it1 = brick.m_ptex_latest_availableTiles[i].begin();
                auto it2 = brick.m_ptex_latest_availableTiles[i].begin();
                std::advance(it2, brick.m_ptex_update_bin_sizes[i]);
                brick.m_ptex_latest_availableTiles[i].erase(it1, it2);
            }

            //TODO THREAD SAFETY 


            auto t_1 = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> task_time = (t_1 - t_0);
            //std::cout << "Building ptex update lists in: " << task_time.count() << std::endl;

            brick.ptex_updateList_computation_time += task_time.count();
            brick.ptex_updated_primitives += tgt_copied_elements;
        }

        void updateTextureTiles(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick)
        {
            GLuint64 t_0, t_1;
            unsigned int queryID[2];
            // generate two queries
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);

            WeakResource<ShaderStorageBufferObject> ptex_bindless_images_handles_resource = resource_mngr.getSSBO(brick.m_ptex_bindless_image_handles);
            WeakResource<ShaderStorageBufferObject> ptex_parameters_backbuffer_resource = resource_mngr.getSSBO(brick.m_ptex_parameters_backbuffer);
            WeakResource<ShaderStorageBufferObject> ptex_parameters_resource = resource_mngr.getSSBO(brick.m_ptex_parameters);
            WeakResource<ShaderStorageBufferObject> ptex_material_bth_resource = resource_mngr.getSSBO(brick.m_ptex_material_bth);
            WeakResource<ShaderStorageBufferObject> updatePatches_SSBO_resource = resource_mngr.getSSBO(brick.m_ptex_updatePatches_tgt_SSBO);
            WeakResource<ShaderStorageBufferObject> availableTiles_SSBO_resource = resource_mngr.getSSBO(brick.m_ptex_availableTiles_SSBO);

            // check availabilty of resources and abort update if any resource is not ready
            bool ptex_update_ready = ptex_bindless_images_handles_resource.state == ResourceState::READY &&
                ptex_parameters_backbuffer_resource.state == ResourceState::READY &&
                ptex_parameters_resource.state == ResourceState::READY &&
                ptex_material_bth_resource.state == ResourceState::READY &&
                updatePatches_SSBO_resource.state == ResourceState::READY &&
                updatePatches_SSBO_resource.state == ResourceState::READY;

            uint update_patches = 0;
            for (auto bin_size : brick.m_ptex_update_bin_sizes)
                update_patches += bin_size;

            bool lod_update_required = false;
            for (int i = 0; i < brick.m_ptex_update_bin_sizes.size(); ++i)
            {
                lod_update_required = lod_update_required || (brick.m_ptex_update_bin_sizes[i] > (i * 16));
            }

            //ptex_update_ready = ptex_update_ready && (brick.m_ptex_update_bin_sizes.size() > 0);
            ptex_update_ready = ptex_update_ready && (update_patches > 0);

            brick.m_ptex_ready = true;

            if (!ptex_update_ready) return;

            glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

            // copy previous ptex params to backbuffer
            //ShaderStorageBufferObject::copy(ptex_parameters_resource.resource, ptex_parameters_backbuffer_resource.resource);

            // update active patch lod classification to latest (the result of which should be used in this call)
            brick.m_ptex_active_patch_lod_classification = brick.m_ptex_latest_patch_lod_classification;
            brick.m_ptex_active_availableTiles = brick.m_ptex_latest_availableTiles;

            // upload update information to GPU
            updatePatches_SSBO_resource.resource->reload(brick.m_ptex_updatePatches_tgt);
            availableTiles_SSBO_resource.resource->reload(brick.m_ptex_availableTiles_uploadBuffer);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


            glQueryCounter(queryID[1], GL_TIMESTAMP);

            // wait until the results are available
            GLint stopTimerAvailable = 0;
            while (!stopTimerAvailable)
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

            // get query results
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

            brick.ptex_tileUpdate_time += (t_1 - t_0) / 1000000.0;

            // TODO per LOD level dispatch computes
            int update_patch_offset = 0;
            int texture_slot_offset = 0;
            int tile_size_multiplier = std::pow(2, brick.m_lod_lvls - 1);

            for (int i = 0; i < static_cast<int>(brick.m_ptex_update_bin_sizes.size()) - 1; ++i)
            {
                // available textures should always be >= update patches per LOD
                assert(brick.m_ptex_availableTiles_bin_sizes[i] >= brick.m_ptex_update_bin_sizes[i]);

                uint32_t bin_size = brick.m_ptex_update_bin_sizes[i];
                float texture_lod = static_cast<float>(i);

                uint32_t remaining_tex_bin_size = brick.m_ptex_availableTiles_bin_sizes[i];

                GLuint64 t_0, t_1;
                unsigned int queryID[2];
                // generate two queries
                glGenQueries(2, queryID);
                glQueryCounter(queryID[0], GL_TIMESTAMP);

                // set GLSL program
                updatePtexTiles_prgm->use();

                // Bind vertex and index buffer as storage buffer
                //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, brick.m_surface_mesh->getVboHandle());
                //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, brick.m_surface_mesh->getIboHandle());

                brick.m_surface_mesh->getVbo().bindAs(GL_SHADER_STORAGE_BUFFER, 0);
                brick.m_surface_mesh->getIbo().bindAs(GL_SHADER_STORAGE_BUFFER, 1);

                ptex_bindless_images_handles_resource.resource->bind(2);
                ptex_parameters_resource.resource->bind(3);

                ptex_material_bth_resource.resource->bind(5);

                updatePatches_SSBO_resource.resource->bind(6);
                availableTiles_SSBO_resource.resource->bind(7);

                ResourceID decal_buffer = GRenderingComponents::decalManager().getGPUBufferResource();
                auto decal_buffer_rsrc = resource_mngr.getSSBO(decal_buffer);
                decal_buffer_rsrc.resource->bind(8);

                updatePtexTiles_prgm->setUniform("decal_cnt", GRenderingComponents::decalManager().getComponentCount());
                updatePtexTiles_prgm->setUniform("texture_lod", texture_lod + 1.0f); //TODO more accurate computation of fitting mipmap level for source textures
                updatePtexTiles_prgm->setUniform("update_patch_offset", update_patch_offset);
                updatePtexTiles_prgm->setUniform("texture_slot_offset", texture_slot_offset);

                updatePtexTiles_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, bin_size);

                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                glQueryCounter(queryID[1], GL_TIMESTAMP);

                // wait until the results are available
                GLint stopTimerAvailable = 0;
                while (!stopTimerAvailable)
                    glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

                // get query results
                glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &t_0);
                glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &t_1);

                brick.ptex_tileUpdate_time += (t_1 - t_0) / 1000000.0;

                update_patch_offset += bin_size;
                texture_slot_offset += bin_size;

                remaining_tex_bin_size -= bin_size;

                texture_slot_offset += remaining_tex_bin_size;

                tile_size_multiplier /= 2;
            }


            // Assign vista tiles (no recomutation necessary)
            if (brick.m_ptex_update_bin_sizes.back() > 0)
            {
                setPtexVistaTiles_prgm->use();

                ptex_parameters_resource.resource->bind(3);
                updatePatches_SSBO_resource.resource->bind(6);

                setPtexVistaTiles_prgm->setUniform("update_patch_offset", update_patch_offset);

                int texture_base_idx = brick.m_ptex_textures.size() - (brick.m_ptex_lod_bin_sizes.back() * 4) / 2048;
                setPtexVistaTiles_prgm->setUniform("texture_base_idx", texture_base_idx);
                setPtexVistaTiles_prgm->setUniform("vista_patch_cnt", brick.m_ptex_update_bin_sizes.back());

                setPtexVistaTiles_prgm->dispatchCompute(1, 1, (brick.m_ptex_update_bin_sizes.back() / 32) + 1);

                //	for (auto texture : brick.m_ptex_textures)
                    //	{
                    //		auto tex_rsrc = resource_mngr.getTexture2DArray(texture);
                    //		tex_rsrc.resource->updateMipmaps();
                    //	}
            }

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // copy updated ptex params to cpu
            size_t byte_size = ptex_parameters_resource.resource->getSize();
            //brick.m_mesh_ptex_params.resize(quad_cnt); // 1 set of ptex params per quad
            ptex_parameters_resource.resource->bind();
            GLvoid* ptex_params = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byte_size, GL_MAP_READ_BIT);
            memcpy(brick.m_mesh_ptex_params.data(), ptex_params, byte_size);
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


            // update average and maximum time
            brick.ptex_distance_computation_avg_time += brick.ptex_distance_computation_time;
            brick.ptex_distance_computation_max_time = std::max(brick.ptex_distance_computation_max_time, brick.ptex_distance_computation_time);
            brick.ptex_distance_computation_time = 0.0;

            brick.ptex_updateList_computation_avg_time += brick.ptex_updateList_computation_time;
            brick.ptex_updateList_computation_max_time = std::max(brick.ptex_updateList_computation_max_time, brick.ptex_updateList_computation_time);
            brick.ptex_updateList_computation_time = 0.0;

            brick.ptex_tileUpdate_avg_time += brick.ptex_tileUpdate_time;
            brick.ptex_tileUpdate_max_time = std::max(brick.ptex_tileUpdate_max_time, brick.ptex_tileUpdate_time);
            brick.ptex_tileUpdate_time = 0.0;

            brick.ptex_updated_primitives_avg += brick.ptex_updated_primitives;
            brick.ptex_updated_primitives_max = std::max(brick.ptex_updated_primitives_max, brick.ptex_updated_primitives);
            brick.ptex_updated_primitives = 0;

            ++brick.updates_made;

            // sum up measurements...output average and max time once a whole of 5seconds of update time has been gathered
            if ((brick.ptex_distance_computation_avg_time + brick.ptex_updateList_computation_avg_time + brick.ptex_tileUpdate_avg_time) > 5000)
            {
                double ptex_update_avg_time = (brick.ptex_distance_computation_avg_time + brick.ptex_updateList_computation_avg_time + brick.ptex_tileUpdate_avg_time) / brick.updates_made;
                double ptex_update_max_time = brick.ptex_distance_computation_max_time + brick.ptex_updateList_computation_max_time + brick.ptex_tileUpdate_max_time;
                std::cout << "============================" << std::endl;
                std::cout << "Ptex Distance Computation (avg): " << brick.ptex_distance_computation_avg_time / brick.updates_made << std::endl;
                std::cout << "Ptex Distance Computation (max): " << brick.ptex_distance_computation_max_time << std::endl;
                std::cout << "Ptex Classification (avg): " << brick.ptex_updateList_computation_avg_time / brick.updates_made << std::endl;
                std::cout << "Ptex Classification (max): " << brick.ptex_updateList_computation_max_time << std::endl;
                std::cout << "Ptex Texture Update (avg): " << brick.ptex_tileUpdate_avg_time / brick.updates_made << std::endl;
                std::cout << "Ptex Texture Update (max): " << brick.ptex_tileUpdate_max_time << std::endl;
                std::cout << "Ptex Update Average Time: " << ptex_update_avg_time << std::endl;
                std::cout << "Ptex Update Max Time: " << ptex_update_max_time << std::endl;
                std::cout << "Ptex Primitives (avg): " << brick.ptex_updated_primitives_avg / brick.updates_made << std::endl;
                std::cout << "Ptex Primitives (max): " << brick.ptex_updated_primitives_max << std::endl;
                std::cout << "============================" << std::endl;

                brick.ptex_distance_computation_avg_time = 0.0;
                brick.ptex_distance_computation_max_time = 0.0;
                brick.ptex_updateList_computation_avg_time = 0.0;
                brick.ptex_updateList_computation_max_time = 0.0;
                brick.ptex_tileUpdate_avg_time = 0.0;
                brick.ptex_tileUpdate_max_time = 0.0;
                brick.ptex_updated_primitives_avg = 0;
                brick.ptex_updated_primitives_max = 0;

                brick.updates_made = 0;
            }

            //TODO add GPU task for mipmap computation
            //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, index]()
            //{
            //if (brick.m_cancel_ptex_update)
            //{
            //	return;
            //}
            {
                WeakResource<ShaderStorageBufferObject> texture_handles_rsrc = resource_mngr.getSSBO(brick.m_ptex_bindless_texture_handles);
                WeakResource<ShaderStorageBufferObject> mipmap_image_handles_rsrc = resource_mngr.getSSBO(brick.m_ptex_bindless_mipmap_image_handles);
                WeakResource<ShaderStorageBufferObject> ptex_params_rsrc = resource_mngr.getSSBO(brick.m_ptex_parameters);
                WeakResource<ShaderStorageBufferObject> updatePatches_rsrc = resource_mngr.getSSBO(brick.m_ptex_updatePatches_tgt_SSBO);

                texture_handles_rsrc.resource->bind(0);
                mipmap_image_handles_rsrc.resource->bind(1);
                ptex_params_rsrc.resource->bind(2);
                updatePatches_rsrc.resource->bind(3);

                int update_patch_offset = 0;
                int tile_size_multiplier = std::pow(2, brick.m_lod_lvls - 1);

                updatePtexTilesMipmaps_prgm->use();

                // only update mipmaps of non-vista level tiles
                for (int i = 0; i < static_cast<int>(brick.m_ptex_update_bin_sizes.size()) - 1; ++i)
                {
                    uint32_t bin_size = brick.m_ptex_update_bin_sizes[i];

                    updatePtexTilesMipmaps_prgm->setUniform("update_patch_offset", update_patch_offset);

                    updatePtexTilesMipmaps_prgm->dispatchCompute(tile_size_multiplier, tile_size_multiplier, bin_size);

                    tile_size_multiplier /= 2;
                    update_patch_offset += bin_size;
                }
            }
            //});

            // clear update bin size to avoid as updates of the latest classification have been applied
            brick.m_ptex_update_bin_sizes.clear();

        }

        void fillSurfaceMeshGaps(uint index)
        {
            // TOOD Find pair of neighbouring bricks

            // TODO Find relevant textures

            // TODO Find required resolution

            // TODO per pair

                // TODO Dispatch compute call to count required triangles

                // TODO Resize mesh buffer

                // TODO Create mesh from compute shader
        }

        void exportMesh(Graphics::OpenGL::ResourceManager& resource_mngr,
            LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent& brick,
            std::string export_filepath)
        {
            // use transformFeedback to write tesselated terrain surface to GPU-buffer

            // allocate buffer size
            size_t byte_size = brick.m_surface_mesh->getIndicesCount() * 3 * sizeof(float) * 2 * 10; //Last multiplication factor is the amount of tessellated triangles
            glBindBuffer(GL_ARRAY_BUFFER, transformFeedback_terrainBuffer);
            glBufferData(GL_ARRAY_BUFFER, byte_size, 0, GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnable(GL_RASTERIZER_DISCARD);

            transformFeedback_terrainOutput_prgm->use();

            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transformFeedback_terrainBuffer);
            glBeginTransformFeedback(GL_TRIANGLES);

            brick.m_surface_mesh->draw();

            glEndTransformFeedback();
            glDisable(GL_RASTERIZER_DISCARD);


            // read GPU-buffer to CPU memory
            float* mesh_data = new float[byte_size / sizeof(float)];

            glBindBuffer(GL_ARRAY_BUFFER, transformFeedback_terrainBuffer);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, byte_size, mesh_data);

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            std::cout << "Vertex 0: " << mesh_data[0] << " " << mesh_data[1] << " " << mesh_data[2] << std::endl;

            // create output obj file from GPU-buffer
            std::ofstream file(export_filepath, std::ios::out | std::ios::binary);

            file << "# Space-Lion Terrain Export\n";
            file << "o Terrain\n";

            for (uint i = 0; i < (byte_size / 4); i = i + 6)
            {
                file << "v " << mesh_data[i] << " " << mesh_data[i + 1] << " " << mesh_data[i + 2] << "\n";
            }

            for (uint i = 0; i < (byte_size / 4); i = i + 6)
            {
                file << "vn " << mesh_data[i + 3] << " " << mesh_data[i + 4] << " " << mesh_data[i + 5] << "\n";
            }

            file << "usemtl None\n";
            file << "s off\n";

            uint vertex_counter = 1;
            for (uint i = 0; i < (byte_size / 4); i = i + 6 * 3)
            {
                file << "f " << vertex_counter << "//" << vertex_counter << " " << vertex_counter + 1 << "//" << vertex_counter + 1 << " " << vertex_counter + 2 << "//" << vertex_counter + 2 << "\n";

                vertex_counter = vertex_counter + 3;
            }

            file.close();
        }

        void addDebugVolume(uint index)
        {
            //////////////////////
            // Create Debug Volume
            //////////////////////

            glUseProgram(0);

            brick.m_normals->bindTexture();
            std::vector<float> volume_data(brick.m_normals->getWidth()
                * brick.m_normals->getHeight()
                * brick.m_normals->getDepth() * 4);
            glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, volume_data.data());
            glowl::TextureLayout debug_volume_descriptor(GL_RGBA32F, brick.m_normals->getWidth(), brick.m_normals->getHeight(), brick.m_normals->getDepth(), GL_RGBA, GL_FLOAT, 1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
                    std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});


            //cube_indices.bindTexture();
            //histogram_pyramid[5]->bindTexture();
            //brick.m_surface->bindTexture();
            //std::vector<GLhalf> volume_data( brick.m_surface->getWidth() * brick.m_surface->getHeight() * brick.m_surface->getDepth() );
            //glGetTexImage(GL_TEXTURE_3D,0,GL_RED,GL_HALF_FLOAT,volume_data.data());
            //TextureDescriptor debug_volume_descriptor(GL_R16F,brick.m_surface->getWidth(),brick.m_surface->getHeight(),brick.m_surface->getDepth(),GL_RED,GL_HALF_FLOAT);

            //brick.m_noise_params->bindTexture();
            //std::vector<GLhalf> volume_data( 2 * brick.m_noise_params->getWidth()*brick.m_noise_params->getHeight()*brick.m_noise_params->getDepth());
            //glGetTexImage(GL_TEXTURE_3D, 0, GL_RG, GL_HALF_FLOAT, volume_data.data());
            //TextureDescriptor debug_volume_descriptor(GL_RG16F, brick.m_noise_params->getWidth(), brick.m_noise_params->getHeight(), brick.m_noise_params->getDepth(), GL_RG, GL_HALF_FLOAT);

            float x = brick.m_dimensions.x / 2.0f;
            float y = brick.m_dimensions.y / 2.0f;
            float z = brick.m_dimensions.z / 2.0f;
            std::vector<float> debug_volume_bg_vertices({ -x,-y,-z,0.0,0.0,0.0,
                -x,y,-z,0.0,1.0,0.0,
                x,y,-z,1.0,1.0,0.0,
                x,-y,-z,1.0,0.0,0.0,
                -x,-y,z,0.0,0.0,1.0,
                -x,y,z,0.0,1.0,1.0,
                x,y,z,1.0,1.0,1.0,
                x,-y,z,1.0,0.0,1.0, });
            std::vector<uint> debug_volume_bg_indices({ 0,2,1,0,3,2, //back face
                0,1,4,1,5,4,
                4,5,7,7,5,6,
                7,6,3,3,2,6,
                5,1,6,6,1,2,
                4,7,0,0,7,3 });
            VertexLayout debug_volume_bg_vertexDesc(24, { VertexLayout::Attribute(GL_FLOAT,3,false,0),VertexLayout::Attribute(GL_FLOAT,3,false,3 * sizeof(GLfloat)) });

            GRenderingComponents::volumeManager().addComponent(brick.m_entity, "brick" + std::to_string(brick.m_entity.id()) + "_volume",
                volume_data, debug_volume_descriptor,
                debug_volume_bg_vertices, debug_volume_bg_indices, debug_volume_bg_vertexDesc, GL_TRIANGLES,
                Vec3(-x, -y, -z), Vec3(x, y, z), false);
        }

        void updateDebugVolume(uint index)
        {
            glUseProgram(0);

            glowl::Texture3D* active_debug_field;

            switch (brick.m_debugField_selection)
            {
            case Datafield::NORMAL:
                active_debug_field = brick.m_normals;
                break;
            case Datafield::GRADIENT:
                active_debug_field = brick.m_gradients;
                break;
            case Datafield::NOISE:
                active_debug_field = brick.m_noise_params;
                break;
            case Datafield::SURFACE:
                active_debug_field = brick.m_surface;
                break;
            case Datafield::SURFACE_BOUNDARY:
                active_debug_field = brick.m_surface_boundaryRegion;
                break;
            default:
                active_debug_field = brick.m_normals;
                break;
            }

            active_debug_field->bindTexture();
            std::vector<float> volume_data(active_debug_field->getWidth() *
                active_debug_field->getHeight() *
                active_debug_field->getDepth() * 4);
            glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, volume_data.data());
            glowl::TextureLayout debug_volume_descriptor(GL_RGBA32F, active_debug_field->getWidth(), active_debug_field->getHeight(), active_debug_field->getDepth(), GL_RGBA, GL_FLOAT, 1,
                { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
                    std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_NEAREST),
                    std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_NEAREST) }, {});

            GRenderingComponents::volumeManager().updateComponent(brick.m_entity, volume_data, debug_volume_descriptor);
        }
    }

    void updateBricks(
        Graphics::Landscape::FeatureCurveComponentManager<Graphics::OpenGL::ResourceManager>& feature_curve_mngr,
        Graphics::OpenGL::GraphicsBackend& graphics_backend,
        Graphics::OpenGL::ResourceManager& resource_mngr,
        Common::TransformComponentManager& transform_mngr,
        LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>& lscp_brick_mngr,
        std::vector<LandscapeBrickComponentManager<Graphics::OpenGL::ResourceManager>::LandscapeBrickComponent>& landscape_bricks,
        std::vector<Entity> const& brick_entities)
    {
        std::vector<uint> brick_indices;
        std::vector<uint> empty_bricks;
        std::vector<uint> brick_iterations;
        uint max_iterations_cnt = 0;

        for (auto brick : brick_entities)
        {
            uint brick_idx = lscp_brick_mngr.getIndex(brick);
            auto brick_res = lscp_brick_mngr.getResolution(brick_idx);

            if (!lscp_brick_mngr.isEmpty(brick_idx))
            {
                brick_indices.push_back(brick_idx);
                brick_iterations.push_back(static_cast<uint>(std::sqrt(
                    brick_res.x * brick_res.x +
                    brick_res.y * brick_res.y +
                    brick_res.z * brick_res.z))
                );

                max_iterations_cnt = std::max(max_iterations_cnt, brick_iterations.back());
            }
        }

        // Reset fields of bricks (both non-empty and empty)
        for (auto brick_idx : brick_indices) {
            auto& brick = landscape_bricks[brick_idx];
            graphics_backend.addSingleExecutionGpuTask([&resource_mngr,&brick] { resetFields(resource_mngr,brick); });
        }
        for (auto brick_idx : empty_bricks) {
            auto& brick = landscape_bricks[brick_idx];
            graphics_backend.addSingleExecutionGpuTask([&resource_mngr, &brick] { resetFields(resource_mngr, brick); });
        }

        // Voxelize Feature Curves in non-empty bricks
        for (auto brick_idx : brick_indices) {
            auto& brick = landscape_bricks[brick_idx];
            graphics_backend.addSingleExecutionGpuTask(
                [&feature_curve_mngr,&resource_mngr,&transform_mngr,&brick] { voxelizeFeatureCurves(feature_curve_mngr,resource_mngr,transform_mngr,brick); }
            );
        }

        // Compute guidance and noise fields
        for (uint i = 0; i < max_iterations_cnt / 2; i++)
        {
            for (auto brick_idx : brick_indices)
            {
                graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeGuidanceField(brick_idx, 2); });
                graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeNoiseField(brick_idx, 2); });
            }
        }
        //	for (auto brick_idx : brick_indices)
        //	{
        //		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeGuidanceField(brick_idx, per_brick_iterations); });
        //		GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeNoiseField(brick_idx, per_brick_iterations); });
        //	}


        // Compute surface propagation
        // Note: surface propagation does 2 iterations internally
        for (uint i = 0; i < max_iterations_cnt / 2; i++)
        {
            for (auto brick_idx : brick_indices)
            {
                graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeSurfacePropagation(brick_idx, 1); });
            }
        }
        //for (auto brick_idx : brick_indices)
        //{
        //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx, per_brick_iterations] { GLandscapeComponents::brickManager().computeSurfacePropagation(brick_idx, per_brick_iterations / 2); });
        //}

        // Smooth result of surface propagation
        for (auto& brick_idx : brick_indices)
            graphics_backend.addSingleExecutionGpuTask([brick_idx] { smoothSurfaceField(brick_idx); });

        // Compute surface mesh
        for (auto& brick_idx : brick_indices)
            graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeSurfaceMesh(brick_idx); });

        //for (auto& brick_idx : brick_indices)
        //	GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, brick_idx] { GLandscapeComponents::brickManager().addDebugVolume(brick_idx);
        //																						GLandscapeComponents::landscapeManager().setReady(brick_idx); });

        // Fill bricks without feature curves after all others have been computed
        for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
        {
            for (auto brick_idx : empty_bricks)
            {
                graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeGuidanceField(brick_idx, 2); });
                graphics_backend.addSingleExecutionGpuTask([brick_idx] { computeNoiseField(brick_idx, 2); });
            }
        }

        // Note: surface propagation does 2 iterations internally
        for (uint i = 0; i < max_iterations_cnt / 2; i++) //TODO use number of actually required iterations
        {
            for (auto brick_idx : empty_bricks)
            {
                graphics_backend.addSingleExecutionGpuTask([brick_idx] {computeSurfacePropagation(brick_idx, 1); });
            }
        }

        for (auto& brick_idx : empty_bricks)
            graphics_backend.addSingleExecutionGpuTask([brick_idx] { smoothSurfaceField(brick_idx); });

        for (auto& brick_idx : empty_bricks)
            graphics_backend.addSingleExecutionGpuTask([brick_idx]
                {
                    GLandscapeComponents::brickManager().computeSurfaceMesh(brick_idx);
                });

    }

}
}
}
}