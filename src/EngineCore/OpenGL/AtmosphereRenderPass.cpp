#include "AtmosphereRenderPass.hpp"

#include "AtmosphereComponentManager.hpp"
#include "CameraComponent.hpp"
#include "GeometryBakery.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "PointlightComponent.hpp"
#include "RenderTaskComponentManager.hpp"
#include "OpenGL/ResourceManager.hpp"
#include "SunlightComponentManager.hpp"
#include "TransformComponentManager.hpp"

void EngineCore::Graphics::OpenGL::addAtmosphereRenderPass(Common::Frame& frame, WorldState& world_state, ResourceManager& resource_mngr)
{
    struct AtmospherePassData
    {
        struct AtmosphereData
        {
            Vec3 position;
            Mat4x4 transform;
            Vec3 beta_r;
            Vec3 beta_m;
            float h_r;
            float h_m;
            float min_altitude;
            float max_altitude;
        };

        struct SunData
        {
            Vec3 position;
            float star_radius;
            float lumen;
        };

        std::vector<AtmosphereData> atmosphere_data;
        std::vector<SunData> sun_data;

        Vec3 camera_position;
        
        Mat4x4 view_matrix;
        Mat4x4 proj_matrix;
    };

    struct AtmospherePassResources
    {
        struct AtmosphereComponentResources
        {
            WeakResource<glowl::Texture2D> transmittance_lut;
            WeakResource<glowl::Texture3D> mie_inscatter_lut;
            WeakResource<glowl::Texture3D> rayleigh_inscatter_lut;
        };

        std::vector<AtmosphereComponentResources> atmosphere_resources;

        WeakResource<glowl::GLSLProgram> atmosphere_prgm;
        WeakResource<glowl::Mesh> atmosphere_proxy_mesh;

        WeakResource<glowl::FramebufferObject> gBuffer;
        WeakResource<glowl::FramebufferObject> atmosphere_render_target;
    };

    // Atmosphere pass
    frame.addRenderPass<AtmospherePassData, AtmospherePassResources>("AtmospherePass",
        // data setup phase
        [&frame, &world_state, &resource_mngr](AtmospherePassData& data, AtmospherePassResources& resources) {
            auto& atmosphere_mngr = world_state.get<Graphics::AtmosphereComponentManager<ResourceManager>>();
            auto& cam_mngr = world_state.get<CameraComponentManager>();
            auto& mtl_mngr = world_state.get<MaterialComponentManager<ResourceManager>>();
            auto& mesh_mngr = world_state.get<MeshComponentManager<ResourceManager>>();
            auto& transform_mngr = world_state.get<Common::TransformComponentManager>();
            auto& sunlight_mngr = world_state.get<Graphics::SunlightComponentManager>();

            // set camera matrices
            Entity camera_entity = cam_mngr.getActiveCamera();
            auto camera_idx = cam_mngr.getIndex(camera_entity).front();
            auto camera_transform_idx = transform_mngr.getIndex(camera_entity);

            data.view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));

            //cam_mngr.setAspectRatio(camera_idx, static_cast<float>(frame.m_window_width) / static_cast<float>(frame.m_window_height));
            cam_mngr.setNear(camera_idx, 1.0f);
            cam_mngr.setFar(camera_idx, 6800000.0);
            cam_mngr.updateProjectionMatrix(camera_idx);
            data.proj_matrix = cam_mngr.getProjectionMatrix(camera_idx);
            cam_mngr.setNear(camera_idx, 0.1f);
            cam_mngr.setFar(camera_idx, 1000.0);
            cam_mngr.updateProjectionMatrix(camera_idx);

            data.camera_position = transform_mngr.getWorldPosition(camera_transform_idx);

            // fill atmosphere data
            auto atmosphere_cnt = atmosphere_mngr.getComponentCount();
            for (uint i = 0; i < atmosphere_cnt; ++i)
            {
                auto transform_idx = transform_mngr.getIndex(atmosphere_mngr.getEntity(i));
                data.atmosphere_data.push_back({
                        transform_mngr.getWorldPosition(transform_idx),
                        transform_mngr.getWorldTransformation(transform_idx),
                        atmosphere_mngr.getBetaR(i),
                        atmosphere_mngr.getBetaM(i),
                        atmosphere_mngr.getHR(i),
                        atmosphere_mngr.getHM(i),
                        atmosphere_mngr.getMinAltitude(i),
                        atmosphere_mngr.getMaxAltitude(i)
                    });
            }

            // fill sun data
            auto sunlight_cnt = sunlight_mngr.getComponentCount();
            for (uint i = 0; i < sunlight_cnt; ++i)
            {
                data.sun_data.push_back({
                    transform_mngr.getWorldPosition(sunlight_mngr.getEntity(i)),
                    sunlight_mngr.getStarRadius(i),
                    sunlight_mngr.getLumen(i)
                    });
            }
        },
        // resource setup phase
        [&world_state, &resource_mngr](AtmospherePassData& data, AtmospherePassResources& resources) {

            auto& atmosphere_mngr = world_state.get<Graphics::AtmosphereComponentManager<ResourceManager>>();

            // fill atmosphere resources
            auto atmosphere_cnt = atmosphere_mngr.getComponentCount();
            for (uint i = 0; i < atmosphere_cnt; ++i)
            {
                auto transmittance_lut = resource_mngr.getTexture2DResource(atmosphere_mngr.getTransmittanceLUT(i));
                auto mie_inscatter_lut = resource_mngr.getTexture3DResource(atmosphere_mngr.getMieInscatterLUT(i));
                auto rayleigh_inscatter_lut = resource_mngr.getTexture3DResource(atmosphere_mngr.getRayleighInscatterLUT(i));

                if (transmittance_lut.state != READY) {
                    glowl::TextureLayout transmittance_layout(GL_RGBA32F, 256, 64, 1, GL_RGBA, GL_FLOAT, 1,
                        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                        std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE) }, {});
                    transmittance_lut = resource_mngr.createTexture2D("transmittance_table_" + atmosphere_mngr.getEntity(i).id(), transmittance_layout, nullptr);

                    atmosphere_mngr.setTransmittanceLUT(i,transmittance_lut.id);

                    // compute transmittance
                    {
                        auto transmittance_prgm = resource_mngr.createShaderProgram("atmosphere_transmittance", { {"../space-lion/resources/shaders/sky/transmittance_c.glsl",glowl::GLSLProgram::ShaderType::Compute} }).resource;
                        
                        transmittance_prgm->use();
                        
                        glBindImageTexture(0, transmittance_lut.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                        transmittance_prgm->setUniform("transmittance_tx2D", 0);
                        
                        transmittance_prgm->setUniform("min_altitude", data.atmosphere_data[i].min_altitude);
                        transmittance_prgm->setUniform("max_altitude", data.atmosphere_data[i].max_altitude);
                        transmittance_prgm->setUniform("beta_r", data.atmosphere_data[i].beta_r);
                        transmittance_prgm->setUniform("beta_m", data.atmosphere_data[i].beta_m);
                        transmittance_prgm->setUniform("h_m", data.atmosphere_data[i].h_m);
                        transmittance_prgm->setUniform("h_r", data.atmosphere_data[i].h_r);
                        
                        glDispatchCompute(256, 64, 1);
                        
                        glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    }
                }

                if (mie_inscatter_lut.state != READY || rayleigh_inscatter_lut.state != READY) {
                    glowl::TextureLayout inscatter_layout(GL_RGBA32F, 256, 128, 32, GL_RGBA, GL_FLOAT, 1,
                        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE),
                            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE),
                            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE),
                            std::pair<GLenum,GLenum>(GL_TEXTURE_MIN_FILTER,GL_LINEAR),
                            std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER,GL_LINEAR) }, {});
                    mie_inscatter_lut = resource_mngr.createTexture3D("mie_inscatter_table_" + atmosphere_mngr.getEntity(i).id(), inscatter_layout, nullptr);
                    rayleigh_inscatter_lut = resource_mngr.createTexture3D("rayleigh_inscatter_table_" + atmosphere_mngr.getEntity(i).id(), inscatter_layout, nullptr);

                    atmosphere_mngr.setMieInscatterLUT(i, mie_inscatter_lut.id);
                    atmosphere_mngr.setRayleighInscatterLUT(i, rayleigh_inscatter_lut.id);

                    auto inscatter_single_prgm = resource_mngr.createShaderProgram("inscatter_single", { { "../space-lion/resources/shaders/sky/inscatter_single_c.glsl", glowl::GLSLProgram::ShaderType::Compute } }).resource;
                    
                    inscatter_single_prgm->use();
                    
                    glBindImageTexture(0, rayleigh_inscatter_lut.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                    inscatter_single_prgm->setUniform("rayleigh_inscatter_tx3D", 0);
                    
                    glBindImageTexture(1, mie_inscatter_lut.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                    inscatter_single_prgm->setUniform("mie_inscatter_tx3D", 1);
                    
                    glActiveTexture(GL_TEXTURE2);
                    inscatter_single_prgm->setUniform("transmittance_tx2D", 2);
                    transmittance_lut.resource->bindTexture();
                    
                    inscatter_single_prgm->setUniform("min_altitude", data.atmosphere_data[i].min_altitude);
                    inscatter_single_prgm->setUniform("max_altitude", data.atmosphere_data[i].max_altitude);
                    inscatter_single_prgm->setUniform("beta_r", data.atmosphere_data[i].beta_r);
                    inscatter_single_prgm->setUniform("beta_m", data.atmosphere_data[i].beta_m);
                    inscatter_single_prgm->setUniform("h_m", data.atmosphere_data[i].h_m);
                    inscatter_single_prgm->setUniform("h_r", data.atmosphere_data[i].h_r);
                    
                    glDispatchCompute(8, 128, 32);
                    
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                }

                resources.atmosphere_resources.push_back({ transmittance_lut, mie_inscatter_lut, rayleigh_inscatter_lut});
            }

            // get atmosphere shader
            resources.atmosphere_prgm = resource_mngr.createShaderProgram(
                "atmosphere",
                { 
                    { "../space-lion/resources/shaders/sky/atmosphere_v.glsl", glowl::GLSLProgram::ShaderType::Vertex},
                    { "../space-lion/resources/shaders/sky/atmosphere_f.glsl", glowl::GLSLProgram::ShaderType::Fragment}
                });

            // get atmosphere proxy mesh
            resources.atmosphere_proxy_mesh = resource_mngr.getMeshResource(atmosphere_mngr.getProxyMesh());
            if (resources.atmosphere_proxy_mesh.state != READY) {
                const auto [icosphere_vertex_data, icosphere_index_data, icosphere_vertex_description] = Graphics::createIcoSphere(5);
                std::vector<glowl::VertexLayout> layout;
                for (auto& generic_layout : (*icosphere_vertex_description)) {
                    layout.push_back(resource_mngr.convertGenericGltfVertexLayout(generic_layout));
                }
                
                auto err = glGetError();
                
                resources.atmosphere_proxy_mesh = resource_mngr.createMesh(
                    "atmosphere_boundingSphere",
                    (*icosphere_vertex_data),
                    (*icosphere_index_data),
                    layout, GL_UNSIGNED_INT, GL_TRIANGLES);

                atmosphere_mngr.setProxyMesh(resources.atmosphere_proxy_mesh.id);
            }

            // check for existing gBuffer
            resources.gBuffer = resource_mngr.getFramebufferObject("GBuffer");
            if (resources.gBuffer.state != READY) {
                // panic?
            }

            // get atmosphere render target
            resources.atmosphere_render_target = resource_mngr.getFramebufferObject("atmosphere_rt");
            if(resources.atmosphere_render_target.state != READY)
            {
                resources.atmosphere_render_target = resource_mngr.createFramebufferObject("atmosphere_rt", 1600, 900);
                resources.atmosphere_render_target.resource->createColorAttachment(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
            }

            auto gl_err = glGetError();
            if (gl_err != GL_NO_ERROR)
                std::cerr << "GL error in atmosphere pass : " << gl_err << std::endl;
        },
        // execute phase
        [&frame, & world_state](AtmospherePassData const& data, AtmospherePassResources const& resources) {

            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            resources.atmosphere_render_target.resource->bind();
            glClearColor(50000.0f, 0.0f, 50000.0f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, resources.atmosphere_render_target.resource->getWidth(), resources.atmosphere_render_target.resource->getHeight());
            
            glActiveTexture(GL_TEXTURE0);
            resources.gBuffer.resource->bindColorbuffer(1);
            //m_gBuffer->bindColorbuffer(0);
            
            resources.atmosphere_prgm.resource->use();
            resources.atmosphere_prgm.resource->setUniform("normal_depth_tx2D", 0);
            resources.atmosphere_prgm.resource->setUniform("projection_matrix", data.proj_matrix);
            resources.atmosphere_prgm.resource->setUniform("view_matrix", data.view_matrix);
            resources.atmosphere_prgm.resource->setUniform("camera_position", data.camera_position);
            

            /*	Draw all entities instanced */
            int instance_counter = 0;
            std::string atmosphere_center_uniform;
            std::string max_altitude_uniform;
            std::string min_altitude_uniform;
            std::string model_uniform;
            
            size_t num_entities = data.atmosphere_data.size();
            
            for (size_t i = 0; i < num_entities; i++)
            {
                if (resources.atmosphere_resources[i].mie_inscatter_lut.state != READY ||
                    resources.atmosphere_resources[i].rayleigh_inscatter_lut.state != READY ||
                    resources.atmosphere_resources[i].transmittance_lut.state != READY)
                {
                    break;
                }

                // Prepare data for each sun
                int sun_cnt = data.sun_data.size();
                for (int j = 0; j < sun_cnt; j++)
                {
                    std::string sun_direction_uniform("suns[" + std::to_string(j) + "].sun_direction");
                    std::string sun_luminance_uniform("suns[" + std::to_string(j) + "].sun_luminance");
                    std::string sun_angle_uniform("suns[" + std::to_string(j) + "].sun_angle");

                    auto sun_position = data.sun_data[j].position;
                    auto atmosphere_center = data.atmosphere_data[i].position;
                    resources.atmosphere_prgm.resource->setUniform(
                        sun_direction_uniform.c_str(), 
                        glm::normalize(sun_position - data.camera_position)
                    );

                    // Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
                    float sun_luminance = data.sun_data[j].lumen;
                    float distance = std::sqrt(
                        (sun_position - atmosphere_center).x * (sun_position - atmosphere_center).x +
                        (sun_position - atmosphere_center).y * (sun_position - atmosphere_center).y +
                        (sun_position - atmosphere_center).z * (sun_position - atmosphere_center).z) - data.atmosphere_data[i].max_altitude;
                    sun_luminance = sun_luminance / (4.0f * 3.14f * std::pow(distance, 2.0f));
                    //sun_luminance = 1.6 * std::pow(10,9);

                    resources.atmosphere_prgm.resource->setUniform(sun_luminance_uniform.c_str(), sun_luminance);

                    float sun_angle = asin(data.sun_data[j].star_radius / glm::length(sun_position));

                    resources.atmosphere_prgm.resource->setUniform(sun_angle_uniform.c_str(), sun_angle);

                    //std::cout<<"Luminance: "<<sun_luminance<<std::endl;
                    //std::cout<<"Distance: "<< distance <<std::endl;
                }
                resources.atmosphere_prgm.resource->setUniform("sun_count", sun_cnt);

                Mat4x4 model_matrix = data.atmosphere_data[i].transform;
                model_uniform = ("model_matrix[" + std::to_string(instance_counter) + "]");
                resources.atmosphere_prgm.resource->setUniform(model_uniform.c_str(), model_matrix);
            
                max_altitude_uniform = ("max_altitude[" + std::to_string(instance_counter) + "]");
                resources.atmosphere_prgm.resource->setUniform(max_altitude_uniform.c_str(), data.atmosphere_data[i].max_altitude);
            
                min_altitude_uniform = ("min_altitude[" + std::to_string(instance_counter) + "]");
                resources.atmosphere_prgm.resource->setUniform(min_altitude_uniform.c_str(), data.atmosphere_data[i].min_altitude);
            
                atmosphere_center_uniform = ("atmosphere_center[" + std::to_string(instance_counter) + "]");
                resources.atmosphere_prgm.resource->setUniform(atmosphere_center_uniform.c_str(), data.atmosphere_data[i].position);

                resources.atmosphere_prgm.resource->setUniform("beta_r", data.atmosphere_data[i].beta_r);
                resources.atmosphere_prgm.resource->setUniform("beta_m", data.atmosphere_data[i].beta_m);
                resources.atmosphere_prgm.resource->setUniform("h_m", data.atmosphere_data[i].h_m);
                resources.atmosphere_prgm.resource->setUniform("h_r", data.atmosphere_data[i].h_r);
            
                glActiveTexture(GL_TEXTURE1);
                resources.atmosphere_resources[i].rayleigh_inscatter_lut.resource->bindTexture();
                resources.atmosphere_prgm.resource->setUniform("rayleigh_inscatter_tx3D", 1);
            
                glActiveTexture(GL_TEXTURE2);
                resources.atmosphere_resources[i].mie_inscatter_lut.resource->bindTexture();
                resources.atmosphere_prgm.resource->setUniform("mie_inscatter_tx3D", 2);
            
                glActiveTexture(GL_TEXTURE3);
                resources.atmosphere_resources[i].transmittance_lut.resource->bindTexture();
                resources.atmosphere_prgm.resource->setUniform("transmittance_tx2D", 3);
            
                instance_counter++;
            
                if (instance_counter == 128)
                {
                    resources.atmosphere_proxy_mesh.resource->draw(instance_counter);
                    instance_counter = 0;
                }
            
            }
            
            resources.atmosphere_proxy_mesh.resource->draw(instance_counter);
        }
    );
}
