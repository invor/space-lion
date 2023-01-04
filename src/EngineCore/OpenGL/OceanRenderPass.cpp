#include "OceanRenderPass.hpp"

#include "AtmosphereComponentManager.hpp"
#include "CameraComponent.hpp"
#include "OceanComponent.hpp"
#include "SunlightComponentManager.hpp"
#include "TransformComponentManager.hpp"

void EngineCore::Graphics::OpenGL::addOceanRenderPass(Common::Frame& frame, WorldState& world_state, ResourceManager& resource_mngr)
{
    struct OceanPassData
    {
        struct OceanData
        {
            float ocean_wave_height;
            float ocean_patch_size;
            unsigned int grid_size;
            float simulation_time;
        };

        std::vector<OceanData> component_data;

        std::vector<Vec4> sunlight_data; ///< vec3 position, float intensity

        Mat4x4 view_matrix;
        Mat4x4 proj_matrix;
    };

    struct OceanPassResources
    {
        struct AtmosphereComponentResources
        {
            WeakResource<glowl::Texture2D> transmittance_lut;
            WeakResource<glowl::Texture3D> mie_inscatter_lut;
            WeakResource<glowl::Texture3D> rayleigh_inscatter_lut;
        };

        struct OceanComponentResources
        {
            WeakResource<glowl::Texture2D> gaussian_noise;
            WeakResource<glowl::Texture2D> tilde_h0_of_k;
            WeakResource<glowl::Texture2D> tilde_h0_of_minus_k;
            WeakResource<glowl::Texture2D> spectrum_x_displacement;
            WeakResource<glowl::Texture2D> spectrum_y_displacement;
            WeakResource<glowl::Texture2D> spectrum_z_displacement;
            WeakResource<glowl::Texture2D> twiddle;
            WeakResource<glowl::Texture2D> ifft_x_a;
            WeakResource<glowl::Texture2D> ifft_x_b;
            WeakResource<glowl::Texture2D> ifft_y_a;
            WeakResource<glowl::Texture2D> ifft_y_b;
            WeakResource<glowl::Texture2D> ifft_z_a;
            WeakResource<glowl::Texture2D> ifft_z_b;
            WeakResource<glowl::Texture2D> displacement;
            WeakResource<glowl::Texture2D> normal;
        };

        std::vector<OceanComponentResources> component_resources;

        WeakResource<glowl::GLSLProgram> twiddle_precompute_prgm;
        WeakResource<glowl::GLSLProgram> intial_spectrum_prgm;
        WeakResource<glowl::GLSLProgram> spectrum_prgm;
        WeakResource<glowl::GLSLProgram> ifft_prgm;
        WeakResource<glowl::GLSLProgram> inversion_prgm;
        WeakResource<glowl::GLSLProgram> displacement_prgm;
        WeakResource<glowl::GLSLProgram> compute_normal_prgm;

        WeakResource<glowl::GLSLProgram> ocean_surface_prgm;
        WeakResource<glowl::Mesh> ocean_surface_mesh;
        WeakResource<glowl::Texture2D> ocean_fresnel_lut;

        std::vector<AtmosphereComponentResources> atmosphere_resources;

        WeakResource<glowl::FramebufferObject> gBuffer;
        WeakResource<glowl::FramebufferObject> ocean_rt;
    };

    frame.addRenderPass<OceanPassData, OceanPassResources>("OceanPass",
        // data setup phase
        [&world_state, &resource_mngr, &frame](OceanPassData& data, OceanPassResources& resources){
            auto const& cam_mngr = world_state.get<CameraComponentManager>();
            auto & ocean_mngr = world_state.get<OceanComponentManager>();
            auto const& sunlight_mngr = world_state.get<Graphics::SunlightComponentManager>();
            auto const& transform_mngr = world_state.get<Common::TransformComponentManager>();

            // set camera matrices
            Entity camera_entity = cam_mngr.getActiveCamera();
            auto camera_idx = cam_mngr.getIndex(camera_entity).front();
            auto camera_transform_idx = transform_mngr.getIndex(camera_entity);

            data.view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));
            data.proj_matrix = cam_mngr.getProjectionMatrix(camera_idx);

            // gather component data
            size_t ocean_component_cnt = ocean_mngr.getComponentCount();
            data.component_data.resize(ocean_component_cnt);
            for (size_t i = 0; i < ocean_component_cnt; ++i) {
                data.component_data[i].grid_size = ocean_mngr.getGridSize(i);
                data.component_data[i].ocean_patch_size = ocean_mngr.getPatchSize(i);
                data.component_data[i].ocean_wave_height = ocean_mngr.getWaveHeight(i);
                data.component_data[i].simulation_time = ocean_mngr.getSimulationTime(i) + frame.m_simulation_dt;
                ocean_mngr.setSimulationTime(i, data.component_data[i].simulation_time);
            }

            uint sunlight_cnt = sunlight_mngr.getComponentCount();
            data.sunlight_data.reserve(sunlight_cnt);
            for (int i = 0; i < sunlight_cnt; i++)
            {
                if (true) // if active
                {
                    Vec3 position = transform_mngr.getWorldPosition(sunlight_mngr.getEntity(i));
                    float intensity = sunlight_mngr.getLumen(i);
                    data.sunlight_data.push_back(Vec4(position, intensity));
                }
            }

            // try to get resources early
            resources.gBuffer = resource_mngr.getFramebufferObject("GBuffer");
            resources.ocean_rt = resource_mngr.getFramebufferObject("ocean_rt");
        },
        // resource setup phase
        [&world_state, &resource_mngr, &frame](OceanPassData& data, OceanPassResources& resources) {
            auto& atmosphere_mngr = world_state.get<Graphics::AtmosphereComponentManager<ResourceManager>>();
            auto& ocean_mngr = world_state.get<OceanComponentManager>();

            if (resources.gBuffer.state != READY) {
                resources.gBuffer = resource_mngr.getFramebufferObject("GBuffer");
            }

            if (resources.ocean_rt.state != READY) {
                resources.ocean_rt = resource_mngr.createFramebufferObject("ocean_rt", 1600, 900);
                resources.ocean_rt.resource->createColorAttachment(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
            }

            // get ocean shaders
            try
            {
                resources.twiddle_precompute_prgm = resource_mngr.createShaderProgram(
                    "ocean_twiddle_precompute", 
                    { { "../space-lion/resources/shaders/ocean/ocean_twiddle_precompute_c.glsl", glowl::GLSLProgram::ShaderType::Compute } }
                );
                resources.intial_spectrum_prgm = resource_mngr.createShaderProgram(
                    "ocean_intial_spectrum",
                    { { "../space-lion/resources/shaders/ocean/ocean_initial_spectrum_c.glsl", glowl::GLSLProgram::ShaderType::Compute } }
                );
                resources.spectrum_prgm = resource_mngr.createShaderProgram(
                    "ocean_spectrum",
                    { {"../space-lion/resources/shaders/ocean/ocean_spectrum_c.glsl", glowl::GLSLProgram::ShaderType::Compute } });
                resources.ifft_prgm = resource_mngr.createShaderProgram(
                    "ocean_ifft",
                    { { "../space-lion/resources/shaders/ocean/ocean_ifft_c.glsl", glowl::GLSLProgram::ShaderType::Compute } });
                resources.inversion_prgm = resource_mngr.createShaderProgram(
                    "ocean_inversion",
                    { { "../space-lion/resources/shaders/ocean/ocean_ifft_inversion_c.glsl", glowl::GLSLProgram::ShaderType::Compute } });
                resources.displacement_prgm = resource_mngr.createShaderProgram(
                    "ocean_displacement",
                    { { "../space-lion/resources/shaders/ocean/ocean_write_displacement_c.glsl", glowl::GLSLProgram::ShaderType::Compute } });
                resources.compute_normal_prgm = resource_mngr.createShaderProgram(
                    "ocean_computeNormal",
                    { { "../space-lion/resources/shaders/ocean/ocean_normal_c.glsl", glowl::GLSLProgram::ShaderType::Compute } });

                resources.ocean_surface_prgm = resource_mngr.createShaderProgram(
                    "ocean_surface",
                    { 
                        {"../space-lion/resources/shaders/ocean/ocean_surface_v.glsl", glowl::GLSLProgram::ShaderType::Vertex },
                        {"../space-lion/resources/shaders/ocean/ocean_surface_f.glsl", glowl::GLSLProgram::ShaderType::Fragment } 
                    }
                );
            }
            catch (const glowl::GLSLProgramException& e)
            {
                //TODO
            }


            // get ocean surface mesh
            {
                ResourceID ocean_surface_mesh_rsrc_id = ocean_mngr.getSurfaceMesh();
                resources.ocean_surface_mesh = resource_mngr.getMeshResource(ocean_surface_mesh_rsrc_id);

                if (resources.ocean_surface_mesh.state != READY) {

                    std::vector<float> vertex_data;
                    std::vector<uint> index_data;

                    // Dummy surface mesh
                    //static float vertices[24] = { -10.0,0.0,-10.0, 0.0,1.0,0.0, 0.0,0.0,
                    //								10.0,0.0,-10.0, 0.0,1.0,0.0, 0.0,0.0,
                    //								0.0,0.0,10.0, 0.0,1.0,0.0, 0.0,0.0 };

                    // TODO: make corresponding to grid resolution

                    int subdivs_x = 512;
                    int subdivs_y = 512;

                    std::vector<float> vertices((subdivs_x + 1)* (subdivs_y + 1) * 5);

                    float offsetX = -(1.0f / 2.0f);
                    float offsetY = -(1.0f / 2.0f);
                    float patchSizeX = 1.0f / subdivs_x;
                    float patchSizeY = 1.0f / subdivs_y;

                    for (int i = 0; i <= subdivs_y; i++)
                    {
                        for (int j = 0; j <= subdivs_x; j++)
                        {
                            vertices[(i * (subdivs_x + 1) + j) * 5 + 0] = (offsetX + float(j) * patchSizeX); //x
                            vertices[(i * (subdivs_x + 1) + j) * 5 + 1] = (0.0); //y
                            vertices[(i * (subdivs_x + 1) + j) * 5 + 2] = (offsetY + float(i) * patchSizeY); //z
                            vertices[(i * (subdivs_x + 1) + j) * 5 + 3] = ((float)j / subdivs_x); //u
                            vertices[(i * (subdivs_x + 1) + j) * 5 + 4] = ((float)i / subdivs_y); //v
                        }
                    }

                    std::vector<std::vector<uint8_t>> surface_vertices;

                    surface_vertices.push_back(
                        std::vector<uint8_t>(
                            reinterpret_cast<uint8_t*>(vertices.data()), 
                            reinterpret_cast<uint8_t*>(vertices.data()) + ((subdivs_x + 1) * (subdivs_y + 1) * 5 * 4)
                        )
                    );

                    std::vector<uint32_t> surface_indices;

                    /*
                    patch layout:

                    n---ne
                    |   |
                    c---e

                    */

                    int n = subdivs_x + 1;
                    int ne = subdivs_y + 2;
                    int e = 1;

                    for (int i = 0; i < subdivs_y; i++)
                    {
                        for (int j = 0; j < subdivs_x; j++)
                        {
                            int c = j + i * (subdivs_x + 1);

                            // add indices for one patch
                            surface_indices.push_back(c);
                            surface_indices.push_back(c + n);
                            surface_indices.push_back(c + e);

                            surface_indices.push_back(c + n);
                            surface_indices.push_back(c + ne);
                            surface_indices.push_back(c + e);
                        }
                    }

                    std::vector<glowl::VertexLayout> vertex_desc = { {
                        5 * 4,
                        {
                            glowl::VertexLayout::Attribute(3,GL_FLOAT,GL_FALSE,0),
                            glowl::VertexLayout::Attribute(2,GL_FLOAT,GL_FALSE,sizeof(GLfloat) * 3)
                        }
                    } };
                    resources.ocean_surface_mesh = resource_mngr.createMesh(
                        "ocean_surface_mesh",
                        surface_vertices,
                        surface_indices,
                        vertex_desc,
                        GL_UNSIGNED_INT,
                        GL_TRIANGLES);

                    ocean_mngr.setSurfaceMesh(resources.ocean_surface_mesh.id);
                }
            }

            // get fresnel LUT
            {
                ResourceID ocean_fresnel_lut_rsrc_id = ocean_mngr.getFresnelLUT();
                resources.ocean_fresnel_lut = resource_mngr.getTexture2DResource(ocean_fresnel_lut_rsrc_id);

                if (resources.ocean_fresnel_lut.state != READY) {
                    std::vector<float> fresnel_data(512);

                    for (int i = 0; i < 512; i++)
                    {
                        float VdotN = static_cast<float>(i) / 512.0f;

                        // From "Realistic, Real-Time Rendering of Ocean Waves"
                        //float c = VdotN;
                        //float n_lambda = 1.333f / 1.0f; // ior_air/ior_water
                        //float g = std::sqrt( n_lambda*n_lambda + c*c - 1.0f );
                        //float f = 0.5f * (std::pow(g - c, 2.0f) / std::pow(g + c, 2.0f)) * (1.0f + std::pow(c*(g + c) - 1.0f, 2.0f) / std::pow(c*(g - c) + 1.0f, 2.0f));

                        // Schlick's approcimation
                        float ior_air = 1.0f;
                        float ior_water = 1.333f;
                        float r_0 = std::pow((ior_air - ior_water) / (ior_air + ior_water), 2.0f);
                        float r = r_0 + (1.0f - r_0) * pow(1.0f - VdotN, 5.0f);

                        fresnel_data[i] = r;
                    }

                    glowl::TextureLayout fresnel_lut_layout(GL_R32F, 512, 1, 1, GL_RED, GL_FLOAT, 1,
                        { std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT),
                            std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT) }, {});
                    resources.ocean_fresnel_lut = resource_mngr.createTexture2D("ocean_fresnel_lut", fresnel_lut_layout, fresnel_data.data());

                    ocean_mngr.setFresnelLUT(resources.ocean_fresnel_lut.id);
                }
            }

            size_t ocean_component_cnt = ocean_mngr.getComponentCount();
            resources.component_resources.resize(ocean_component_cnt);
            for (size_t i = 0; i < ocean_component_cnt; ++i) {

                // check ocean component resources
                ResourceID gaussian_noise = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::GAUSSIAN_NOISE);
                ResourceID tilde_h0_of_k = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::TILDE_H0_K);
                ResourceID tilde_h0_of_minus_k = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::TILDE_H0_MINUS_K);
                ResourceID spectrum_x_displacement = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_X_DISPLACEMENT);
                ResourceID spectrum_y_displacement = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_Y_DISPLACEMENT);
                ResourceID spectrum_z_displacement = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_Z_DISPLACEMENT);

                ResourceID twiddle = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::TWIDDLE);
                ResourceID ifft_x_a = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_X_A);
                ResourceID ifft_x_b = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_X_B);
                ResourceID ifft_y_a = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Y_A);
                ResourceID ifft_y_b = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Y_B);
                ResourceID ifft_z_a = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Z_A);
                ResourceID ifft_z_b = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Z_B);

                ResourceID displacement = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::DISPLACEMENT);
                ResourceID normal = ocean_mngr.getTextureResource(i, OceanComponentManager::TextureSemantic::NORMAL);

                glowl::TextureLayout rgba32f_layout(GL_RGBA32F, data.component_data[i].grid_size, data.component_data[i].grid_size, 1, GL_RGBA, GL_FLOAT, 1);
                glowl::TextureLayout rg32f_layout(GL_RG32F, data.component_data[i].grid_size, data.component_data[i].grid_size, 1, GL_RG, GL_FLOAT, 1);

                bool rsrcs_creation_required = (gaussian_noise == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (tilde_h0_of_k == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (tilde_h0_of_minus_k == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (spectrum_x_displacement == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (spectrum_y_displacement == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (spectrum_z_displacement == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (twiddle == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_x_a == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_x_b == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_y_a == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_y_b == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_z_a == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (ifft_z_b == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (displacement == resource_mngr.invalidResourceID());
                rsrcs_creation_required |= (normal == resource_mngr.invalidResourceID());

                // if any resource is not valied, i.e., not created, create all resources
                // in practise all resources for a component should either be all valid or all invalid
                if (rsrcs_creation_required) {
                    // for now create random texture on the CPU
                    std::random_device rd;
                    std::mt19937 mt(rd());
                    std::normal_distribution<float> gaussian_dist(0.0f, 1.0f);
                    std::vector<float> gaussian_noise_data(data.component_data[i].grid_size * data.component_data[i].grid_size * 4, 1.0f);
                    for (int i = 0; i < gaussian_noise_data.size(); i++)
                    {
                        gaussian_noise_data[i] = gaussian_dist(mt);
                    }
                    resources.component_resources[i].gaussian_noise 
                        = resource_mngr.createTexture2D("ocean_gaussian_noise_" + std::to_string(i), rgba32f_layout, gaussian_noise_data.data());
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::GAUSSIAN_NOISE, resources.component_resources[i].gaussian_noise.id);
                
                    resources.component_resources[i].tilde_h0_of_k 
                        = resource_mngr.createTexture2D("ocean_h0_k_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::TILDE_H0_K, resources.component_resources[i].tilde_h0_of_k.id);

                    resources.component_resources[i].tilde_h0_of_minus_k 
                        = resource_mngr.createTexture2D("ocean_h0_-k_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::TILDE_H0_MINUS_K, resources.component_resources[i].tilde_h0_of_minus_k.id);

                    resources.component_resources[i].spectrum_x_displacement 
                        = resource_mngr.createTexture2D("ocean_spectrum_x_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_X_DISPLACEMENT, resources.component_resources[i].spectrum_x_displacement.id);

                    resources.component_resources[i].spectrum_y_displacement
                        = resource_mngr.createTexture2D("ocean_spectrum_y_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_Y_DISPLACEMENT, resources.component_resources[i].spectrum_y_displacement.id);

                    resources.component_resources[i].spectrum_z_displacement
                        = resource_mngr.createTexture2D("ocean_spectrum_z_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::SPECTRUM_Z_DISPLACEMENT, resources.component_resources[i].spectrum_z_displacement.id);

                    resources.component_resources[i].twiddle
                        = resource_mngr.createTexture2D("ocean_twiddle_" + std::to_string(i), rgba32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::TWIDDLE, resources.component_resources[i].twiddle.id);

                    resources.component_resources[i].ifft_x_a
                        = resource_mngr.createTexture2D("ocean_ifft_x_a_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_X_A, resources.component_resources[i].ifft_x_a.id);

                    resources.component_resources[i].ifft_x_b
                        = resource_mngr.createTexture2D("ocean_ifft_x_b_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_X_B, resources.component_resources[i].ifft_x_b.id);

                    resources.component_resources[i].ifft_y_a
                        = resource_mngr.createTexture2D("ocean_ifft_y_a_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Y_A, resources.component_resources[i].ifft_y_a.id);

                    resources.component_resources[i].ifft_y_b
                        = resource_mngr.createTexture2D("ocean_ifft_y_b_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Y_B, resources.component_resources[i].ifft_y_b.id);

                    resources.component_resources[i].ifft_z_a
                        = resource_mngr.createTexture2D("ocean_ifft_z_a_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Z_A, resources.component_resources[i].ifft_z_a.id);

                    resources.component_resources[i].ifft_z_b
                        = resource_mngr.createTexture2D("ocean_ifft_z_b_" + std::to_string(i), rg32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::IFFT_Z_B, resources.component_resources[i].ifft_z_b.id);

                    resources.component_resources[i].displacement
                        = resource_mngr.createTexture2D("ocean_displacement_" + std::to_string(i), rgba32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::DISPLACEMENT, resources.component_resources[i].displacement.id);

                    resources.component_resources[i].normal
                        = resource_mngr.createTexture2D("ocean_normal_" + std::to_string(i), rgba32f_layout, NULL);
                    ocean_mngr.setTextureResource(i, OceanComponentManager::TextureSemantic::NORMAL, resources.component_resources[i].normal.id);

                    // compute initial values for spectra
                    {
                        auto intial_spectrum_prgm = resources.intial_spectrum_prgm.resource;

                        intial_spectrum_prgm->use();

                        glActiveTexture(GL_TEXTURE0);
                        resources.component_resources[i].gaussian_noise.resource->bindTexture();
                        intial_spectrum_prgm->setUniform("gaussian_noise_tx2D", 0);

                        glBindImageTexture(0, resources.component_resources[i].tilde_h0_of_k.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        intial_spectrum_prgm->setUniform("tilde_h0_of_k_tx2D", 0);

                        glBindImageTexture(1, resources.component_resources[i].tilde_h0_of_minus_k.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        intial_spectrum_prgm->setUniform("tilde_h0_of_minus_k_tx2D", 1);

                        // TODO pass an actual wind value
                        intial_spectrum_prgm->setUniform("wind", Vec2(500.0f, 500.0f));
                        intial_spectrum_prgm->setUniform("size", Vec2(data.component_data[i].ocean_patch_size));
                        intial_spectrum_prgm->setUniform("A", data.component_data[i].ocean_wave_height);
                        intial_spectrum_prgm->setUniform("grid_size", data.component_data[i].grid_size);

                        glDispatchCompute(data.component_data[i].grid_size, data.component_data[i].grid_size, 1);
                    }

                    // compute initial values for twiddle
                    {
                        // Bit reversed order of stage 0 indices
                        std::vector<int> bit_reversed_indices(data.component_data[i].grid_size);
                        for (int bit_index = 0; bit_index < bit_reversed_indices.size(); bit_index++)
                        {
                            //CAUTION! This is for 9bit indices
                            int b = 0;

                            if (data.component_data[i].grid_size == 512)
                            {
                                b = ((bit_index & 0x1) != 0) ? b | (0x100) : b;
                                b = ((bit_index & 0x2) != 0) ? b | (0x80) : b;
                                b = ((bit_index & 0x4) != 0) ? b | (0x40) : b;
                                b = ((bit_index & 0x8) != 0) ? b | (0x20) : b;
                                b = ((bit_index & 0x10) != 0) ? b | (0x10) : b;
                                b = ((bit_index & 0x20) != 0) ? b | (0x8) : b;
                                b = ((bit_index & 0x40) != 0) ? b | (0x4) : b;
                                b = ((bit_index & 0x80) != 0) ? b | (0x2) : b;
                                b = ((bit_index & 0x100) != 0) ? b | (0x1) : b;
                            }
                            else if (data.component_data[i].grid_size == 256)
                            {
                                b = ((bit_index & 0x1) != 0) ? b | (0x80) : b;
                                b = ((bit_index & 0x2) != 0) ? b | (0x40) : b;
                                b = ((bit_index & 0x4) != 0) ? b | (0x20) : b;
                                b = ((bit_index & 0x8) != 0) ? b | (0x10) : b;
                                b = ((bit_index & 0x10) != 0) ? b | (0x8) : b;
                                b = ((bit_index & 0x20) != 0) ? b | (0x4) : b;
                                b = ((bit_index & 0x40) != 0) ? b | (0x2) : b;
                                b = ((bit_index & 0x80) != 0) ? b | (0x1) : b;
                            }

                            bit_reversed_indices[bit_index] = b;

                            //std::cout << "Bit reversed indices: " << b << std::endl;
                        }

                        glowl::BufferObject indices_ssbo(GL_SHADER_STORAGE_BUFFER, bit_reversed_indices);

                        resources.twiddle_precompute_prgm.resource->use();

                        glBindImageTexture(0, resources.component_resources[i].twiddle.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                        resources.twiddle_precompute_prgm.resource->setUniform("twiddle_tx2D", 0);
                        resources.twiddle_precompute_prgm.resource->setUniform("grid_size", data.component_data[i].grid_size);

                        indices_ssbo.bind(0);

                        glDispatchCompute(static_cast<GLuint>(std::log2(data.component_data[i].grid_size)), data.component_data[i].grid_size / 2, 1);
                    }
                }
                else
                {
                    resources.component_resources[i].gaussian_noise = resource_mngr.getTexture2DResource(gaussian_noise);

                    resources.component_resources[i].tilde_h0_of_k = resource_mngr.getTexture2DResource(tilde_h0_of_k);

                    resources.component_resources[i].tilde_h0_of_minus_k = resource_mngr.getTexture2DResource(tilde_h0_of_minus_k);

                    resources.component_resources[i].spectrum_x_displacement = resource_mngr.getTexture2DResource(spectrum_x_displacement);

                    resources.component_resources[i].spectrum_y_displacement = resource_mngr.getTexture2DResource(spectrum_y_displacement);

                    resources.component_resources[i].spectrum_z_displacement = resource_mngr.getTexture2DResource(spectrum_z_displacement);

                    resources.component_resources[i].twiddle = resource_mngr.getTexture2DResource(twiddle);

                    resources.component_resources[i].ifft_x_a = resource_mngr.getTexture2DResource(ifft_x_a);

                    resources.component_resources[i].ifft_x_b = resource_mngr.getTexture2DResource(ifft_x_b);

                    resources.component_resources[i].ifft_y_a = resource_mngr.getTexture2DResource(ifft_y_a);

                    resources.component_resources[i].ifft_y_b = resource_mngr.getTexture2DResource(ifft_y_b);

                    resources.component_resources[i].ifft_z_a = resource_mngr.getTexture2DResource(ifft_z_a);

                    resources.component_resources[i].ifft_z_b = resource_mngr.getTexture2DResource(ifft_z_b);

                    resources.component_resources[i].displacement = resource_mngr.getTexture2DResource(displacement);

                    resources.component_resources[i].normal = resource_mngr.getTexture2DResource(normal);
                }

                // update specturm
                {
                    auto spectrum_prgm = resources.spectrum_prgm.resource;

                    spectrum_prgm->use();

                    glActiveTexture(GL_TEXTURE0);
                    resources.component_resources[i].tilde_h0_of_k.resource->bindTexture();
                    spectrum_prgm->setUniform("tilde_h0_of_k_tx2D", 0);

                    glActiveTexture(GL_TEXTURE1);
                    resources.component_resources[i].tilde_h0_of_minus_k.resource->bindTexture();
                    spectrum_prgm->setUniform("tilde_h0_of_minus_k_tx2D", 1);

                    glBindImageTexture(0, resources.component_resources[i].spectrum_x_displacement.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
                    spectrum_prgm->setUniform("spectrum_x_tx2D", 0);
                    glBindImageTexture(1, resources.component_resources[i].spectrum_y_displacement.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
                    spectrum_prgm->setUniform("spectrum_y_tx2D", 1);
                    glBindImageTexture(2, resources.component_resources[i].spectrum_z_displacement.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
                    spectrum_prgm->setUniform("spectrum_z_tx2D", 2);

                    spectrum_prgm->setUniform("T", 30.0f);
                    spectrum_prgm->setUniform("t", data.component_data[i].simulation_time);
                    spectrum_prgm->setUniform("grid_size", data.component_data[i].grid_size);
                    spectrum_prgm->setUniform("size", Vec2(data.component_data[i].ocean_patch_size));

                    glDispatchCompute(data.component_data[i].grid_size / 4, data.component_data[i].grid_size / 4, 1);
                }

                // update ocean height fields by computing inverse FFT
                {
                    auto ifft_prgm = resources.ifft_prgm.resource;

                    ifft_prgm->use();

                    glActiveTexture(GL_TEXTURE0);
                    resources.component_resources[i].twiddle.resource->bindTexture();
                    ifft_prgm->setUniform("twiddle_tx2D", 0);

                    // first ifft stage of first direction - read data from spectra and writ to ifft_*_a

                    ifft_prgm->setUniform("src_x_tx2D", 0);
                    ifft_prgm->setUniform("src_y_tx2D", 1);
                    ifft_prgm->setUniform("src_z_tx2D", 2);

                    ifft_prgm->setUniform("tgt_x_tx2D", 3);
                    ifft_prgm->setUniform("tgt_y_tx2D", 4);
                    ifft_prgm->setUniform("tgt_z_tx2D", 5);

                    glBindImageTexture(0, resources.component_resources[i].spectrum_x_displacement.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(1, resources.component_resources[i].spectrum_y_displacement.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(2, resources.component_resources[i].spectrum_z_displacement.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

                    glBindImageTexture(3, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                    glBindImageTexture(4, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                    glBindImageTexture(5, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

                    ifft_prgm->setUniform("ifft_stage", 0);
                    ifft_prgm->setUniform("ifft_direction", 0);

                    glDispatchCompute(data.component_data[i].grid_size, data.component_data[i].grid_size / 2, 1);

                    int src = 0;

                    //ifft_prgm->setUniform("ifft_direction", 0);

                    for (int j = 1; j < (std::log2(data.component_data[i].grid_size)); j++)
                    {
                        // calling glBindImage expensive? if-query in shader faster?
                        if (src == 0)
                        {
                            glBindImageTexture(0, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(1, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(2, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

                            glBindImageTexture(3, resources.component_resources[i].ifft_x_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(4, resources.component_resources[i].ifft_y_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(5, resources.component_resources[i].ifft_z_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        }
                        else
                        {
                            glBindImageTexture(0, resources.component_resources[i].ifft_x_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(1, resources.component_resources[i].ifft_y_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(2, resources.component_resources[i].ifft_z_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

                            glBindImageTexture(3, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(4, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(5, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        }

                        ifft_prgm->setUniform("ifft_stage", j);

                        glDispatchCompute(data.component_data[i].grid_size / 4, data.component_data[i].grid_size / (2 * 4), 1);

                        src = (src == 0) ? 1 : 0;

                        glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    }


                    ifft_prgm->setUniform("ifft_direction", 1);

                    for (int j = 0; j < (std::log2(data.component_data[i].grid_size)); j++)
                    {
                        // calling glBindImage expensive? if-query in shader faster?
                        if (src == 0)
                        {
                            glBindImageTexture(0, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(1, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(2, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

                            glBindImageTexture(3, resources.component_resources[i].ifft_x_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(4, resources.component_resources[i].ifft_y_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(5, resources.component_resources[i].ifft_z_b.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        }
                        else
                        {
                            glBindImageTexture(0, resources.component_resources[i].ifft_x_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(1, resources.component_resources[i].ifft_y_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
                            glBindImageTexture(2, resources.component_resources[i].ifft_z_b.resource->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

                            glBindImageTexture(3, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(4, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                            glBindImageTexture(5, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
                        }

                        ifft_prgm->setUniform("ifft_stage", j);

                        glDispatchCompute(data.component_data[i].grid_size / 4, data.component_data[i].grid_size / (2 * 4), 1);

                        src = (src == 0) ? 1 : 0;

                        glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    }

                }

                // update displacement map
                {
                    auto displacement_prgm = resources.displacement_prgm.resource;
                    displacement_prgm->use();

                    // log(grid_size) even -> read values from b textures
                    //	if (( static_cast<int>(std::log2(m_data[index].grid_size))) == 1)
                    //	{
                    //		glBindImageTexture(0, m_data[index].ifft_x_b->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //		glBindImageTexture(1, m_data[index].ifft_y_b->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //		glBindImageTexture(2, m_data[index].ifft_z_b->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //	}
                    //	else
                    //	{
                    //		glBindImageTexture(0, m_data[index].ifft_x_a->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //		glBindImageTexture(1, m_data[index].ifft_y_a->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //		glBindImageTexture(2, m_data[index].ifft_z_a->getHandle(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    //	}

                    glBindImageTexture(0, resources.component_resources[i].ifft_x_a.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(1, resources.component_resources[i].ifft_y_a.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(2, resources.component_resources[i].ifft_z_a.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(3, resources.component_resources[i].ifft_x_b.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(4, resources.component_resources[i].ifft_y_b.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
                    glBindImageTexture(5, resources.component_resources[i].ifft_z_b.resource->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);

                    displacement_prgm->setUniform("ifft_x_tx2D", 0);
                    displacement_prgm->setUniform("ifft_y_tx2D", 1);
                    displacement_prgm->setUniform("ifft_z_tx2D", 2);

                    displacement_prgm->setUniform("ifft_x_b_tx2D", 3);
                    displacement_prgm->setUniform("ifft_y_b_tx2D", 4);
                    displacement_prgm->setUniform("ifft_z_b_tx2D", 5);

                    glBindImageTexture(6, resources.component_resources[i].displacement.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                    displacement_prgm->setUniform("displacement_tx2D", 6);

                    displacement_prgm->setUniform("grid_size", data.component_data[i].grid_size);

                    glDispatchCompute(data.component_data[i].grid_size / 4, data.component_data[i].grid_size / 4, 1);

                    auto compute_normal_prgm = resources.compute_normal_prgm.resource;
                    compute_normal_prgm->use();

                    glActiveTexture(GL_TEXTURE0);
                    resources.component_resources[i].displacement.resource->bindTexture();
                    compute_normal_prgm->setUniform("displacemnet_tx2D", 0);

                    glBindImageTexture(0, resources.component_resources[i].normal.resource->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                    compute_normal_prgm->setUniform("normal_tx2D", 0);

                    glDispatchCompute(data.component_data[i].grid_size / 4, data.component_data[i].grid_size / 4, 1);
                }
            }

            // get atmoshphere resources
            auto atmosphere_cnt = atmosphere_mngr.getComponentCount();
            for (uint i = 0; i < atmosphere_cnt; ++i)
            {
                auto transmittance_lut = resource_mngr.getTexture2DResource(atmosphere_mngr.getTransmittanceLUT(i));
                auto mie_inscatter_lut = resource_mngr.getTexture3DResource(atmosphere_mngr.getMieInscatterLUT(i));
                auto rayleigh_inscatter_lut = resource_mngr.getTexture3DResource(atmosphere_mngr.getRayleighInscatterLUT(i));

                resources.atmosphere_resources.push_back({ transmittance_lut, mie_inscatter_lut, rayleigh_inscatter_lut });
            }

        },
        // execute phase
        [](OceanPassData const& data, OceanPassResources const& resources) {

            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            resources.ocean_rt.resource->bind();
            glClearColor(0.0, 0.0f, 0.0f, 0.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, resources.ocean_rt.resource->getWidth(), resources.ocean_rt.resource->getHeight());

            size_t ocean_component_cnt = resources.component_resources.size();
            for (size_t i = 0; i < ocean_component_cnt; ++i) {
            
                auto ocean_surface_prgm = resources.ocean_surface_prgm.resource;
                ocean_surface_prgm->use();
            
                // bind shader program and set per program uniforms
                ocean_surface_prgm->setUniform("projection_matrix", data.proj_matrix);
                ocean_surface_prgm->setUniform("view_matrix", data.view_matrix);
            
                glActiveTexture(GL_TEXTURE0);
                if (resources.component_resources[i].displacement.state == READY)
                    resources.component_resources[i].displacement.resource->bindTexture();
                ocean_surface_prgm->setUniform("displacement_tx2D", 0);
            
                glActiveTexture(GL_TEXTURE1);
                if (resources.component_resources[i].normal.state == READY)
                    resources.component_resources[i].normal.resource->bindTexture();
                ocean_surface_prgm->setUniform("normal_tx2D", 1);
            
                glActiveTexture(GL_TEXTURE2);
                if (resources.ocean_fresnel_lut.state == READY)
                    resources.ocean_fresnel_lut.resource->bindTexture();
                ocean_surface_prgm->setUniform("fresnel_lut_tx2D", 2);
            
                glActiveTexture(GL_TEXTURE3);
                resources.gBuffer.resource->bindColorbuffer(1); // depth value
                ocean_surface_prgm->setUniform("depth_tx2D", 3);
            
                ocean_surface_prgm->setUniform(
                    "gBuffer_resolution",
                    Vec2(resources.gBuffer.resource->getWidth(), resources.gBuffer.resource->getHeight())
                );
            
                // TODO Add sky lighting
                if (!resources.atmosphere_resources.empty()) {
                    glActiveTexture(GL_TEXTURE4);
                    resources.atmosphere_resources[0].rayleigh_inscatter_lut.resource->bindTexture();
                    ocean_surface_prgm->setUniform("atmosphere_rayleigh_scattering", 5);
            
                    glActiveTexture(GL_TEXTURE5);
                    resources.atmosphere_resources[0].mie_inscatter_lut.resource->bindTexture();
                    ocean_surface_prgm->setUniform("atmosphere_mie_scattering", 4);
                }
            
                int sun_count = data.sunlight_data.size();
                for (int i = 0; i < sun_count; i++)
                {
                    std::string sun_position_uniform("suns[" + std::to_string(i) + "].position");
                    std::string sun_illuminance_uniform("suns[" + std::to_string(i) + "].illuminance");
            
                    Vec3 light_position = Vec3(data.sunlight_data[i]);
                    ocean_surface_prgm->setUniform(sun_position_uniform.c_str(), light_position);
            
                    // Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
                    float sun_luminous_power = data.sunlight_data[i].w;
            
                    //TODO compute actual illuminance
            
                    ocean_surface_prgm->setUniform(sun_illuminance_uniform.c_str(), 100000.0f);
                }
            
                ocean_surface_prgm->setUniform("num_suns", sun_count);
            
                ocean_surface_prgm->setUniform("size", data.component_data[i].ocean_patch_size);
                ocean_surface_prgm->setUniform("grid_size", data.component_data[i].grid_size);
            
                resources.ocean_surface_mesh.resource->draw();
            }
        }
    );
}
