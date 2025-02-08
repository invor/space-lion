#ifndef ParticlesRenderPass_hpp
#define ParticlesRenderPass_hpp

#include "Frame.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "ParticlesComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"
#include "ResourceManager.hpp"
#include "TransformComponentManager.hpp"
#include "WorldState.hpp"

namespace EngineCore {
    namespace Graphics {
        namespace Dx11 {

            template<typename FrameType>
            void addParticlesRenderPass(FrameType& frame, WorldState& world_state, ResourceManager& resource_mngr)
            {
                struct Data {
                    struct ParticlesConstantBuffer {
                        Mat4x4 transform;

                        Vec4   padding[12];
                    };

                    struct RenderTaskData
                    {
                        EngineCore::Graphics::ResourceID shader_resource;
                        EngineCore::Graphics::ResourceID particles_resource;

                        unsigned int particles_cnt;
                    };

                    std::vector<ParticlesConstantBuffer> particles_cbs;
                    std::vector<RenderTaskData> particles_rtd;
                };

                struct Resources {
                    struct BatchResources
                    {
                        EngineCore::Graphics::WeakResource<dxowl::ShaderProgram> shader_prgm;
                        Microsoft::WRL::ComPtr<ID3D11Buffer>                     vs_constant_buffer;
                        UINT                                                     vs_constant_buffer_offset;
                        UINT                                                     vs_constant_buffer_constants;
                        EngineCore::Graphics::WeakResource<dxowl::Buffer>        particles_buffer;
                        unsigned int                                             particles_cnt;
                    };

                    ID3D11Device4* d3d11_device;
                    ID3D11DeviceContext4* d3d11_device_context;
                    Microsoft::WRL::ComPtr<ID3D11Buffer> view_proj_buffer;
                    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;

                    std::vector<BatchResources> particles_resources;
                };

                frame.addRenderPass<Data, Resources>("Particles",
                    [&world_state](Data& data, Resources& resources) {
                        auto& mtl_mngr = world_state.get<EngineCore::Graphics::MaterialComponentManager>();
                        auto& particles_renderTask_mngr = world_state.get<EngineCore::Graphics::RenderTaskComponentManager<EngineCore::Graphics::RenderTaskTags::Particles>>();
                        auto& transform_mngr = world_state.get<EngineCore::Common::TransformComponentManager>();
                        auto& particles_mngr = world_state.get<EngineCore::Graphics::ParticlesComponentManager<EngineCore::Graphics::Dx11::ResourceManager>>();

                        auto rts = particles_renderTask_mngr.getComponentDataCopy();

                        for (auto const& rt : rts)
                        {
                            if (rt.visible) {
                                data.particles_cbs.push_back(Data::ParticlesConstantBuffer());

                                auto transform_idx = transform_mngr.getIndex(rt.entity);
                                if (transform_idx < (std::numeric_limits<size_t>::max)())
                                {
                                    data.particles_cbs.back().transform = glm::transpose(transform_mngr.getWorldTransformation(transform_idx));
                                }

                                auto particles_indices = particles_mngr.getIndex(rt.entity);
                                EngineCore::Graphics::ResourceID particles_rsrc;
                                unsigned int particles_cnt = 0;
                                if (!particles_indices.empty()) {
                                    particles_rsrc = particles_mngr.getComponent(particles_indices.front()).particle_data;
                                    particles_cnt = static_cast<unsigned int>(particles_mngr.getComponent(particles_indices.front()).particle_count);
                                }

                                data.particles_rtd.push_back({ rt.shader_prgm, particles_rsrc,particles_cnt });
                            }
                        }
                    },
                    [&frame, &resource_mngr](Data& data, Resources& resources) {

                        // obtain access to device resources
                        resources.d3d11_device = resource_mngr.getD3D11Device();
                        resources.d3d11_device_context = resource_mngr.getD3D11DeviceContext();

                        frame.createViewProjectionConstantBuffer(resources.d3d11_device, resources.view_proj_buffer);

                        //TODO why not use dxowl?
                        Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer(nullptr);
                        if (data.particles_cbs.size() > 0) {
                            D3D11_SUBRESOURCE_DATA constantBufferData = { 0 };
                            constantBufferData.pSysMem = data.particles_cbs.data();
                            constantBufferData.SysMemPitch = 0;
                            constantBufferData.SysMemSlicePitch = 0;
                            const CD3D11_BUFFER_DESC constantBufferDesc(
                                static_cast<UINT>(sizeof(Data::ParticlesConstantBuffer) * data.particles_cbs.size()),
                                D3D11_BIND_CONSTANT_BUFFER
                            );
                            winrt::check_hresult(
                                resources.d3d11_device->CreateBuffer(
                                    &constantBufferDesc,
                                    &constantBufferData,
                                    &constant_buffer
                                ));
                        }

                        resources.particles_resources.reserve(data.particles_cbs.size());

                        for (size_t rt_idx = 0; rt_idx < data.particles_rtd.size(); ++rt_idx)
                        {
                            EngineCore::Graphics::WeakResource<dxowl::ShaderProgram> shader_prgm = resource_mngr.getShaderProgramResource(data.particles_rtd[rt_idx].shader_resource);
                            EngineCore::Graphics::WeakResource<dxowl::Buffer> particles_buffer = resource_mngr.getBufferResource(data.particles_rtd[rt_idx].particles_resource);

                            resources.particles_resources.push_back(
                                {
                                    shader_prgm,
                                    constant_buffer,
                                    static_cast<UINT>(rt_idx * sizeof(Data::ParticlesConstantBuffer) / sizeof(Vec4)),
                                    static_cast<UINT>(sizeof(Data::ParticlesConstantBuffer) / sizeof(Vec4)),
                                    particles_buffer,
                                    data.particles_rtd[rt_idx].particles_cnt
                                }
                            );
                        }
                    },
                    [&frame, &resource_mngr](Data const& data, Resources const& resources) {
                        // obtain context and device for rendering
                        const auto context = resources.d3d11_device_context;
                        const auto device = resources.d3d11_device;

                        // track mesh and shader object to minimize binding changes
                        EngineCore::Graphics::ResourceID current_mesh = resource_mngr.invalidResourceID();
                        EngineCore::Graphics::ResourceID current_shader = resource_mngr.invalidResourceID();

                        // loop over all render tasks
                        {
                            size_t rt_cnt = resources.particles_resources.size();
                            for (size_t rt_idx = 0; rt_idx < rt_cnt; ++rt_idx)
                            {
                                if (resources.particles_resources[rt_idx].shader_prgm.state == EngineCore::Graphics::READY
                                    && resources.particles_resources[rt_idx].particles_buffer.state == EngineCore::Graphics::READY)
                                {
                                    dxowl::ShaderProgram* shader = resources.particles_resources[rt_idx].shader_prgm.resource;

                                    if (current_shader.value() != resources.particles_resources[rt_idx].shader_prgm.id.value())
                                    {
                                        current_shader = resources.particles_resources[rt_idx].shader_prgm.id;

                                        shader->setInputLayout(context);

                                        shader->setVertexShader(context);

                                        shader->setGeometryShader(context);

                                        shader->setPixelShader(context);

                                        // Apply the view projection constant buffer to shaders
                                        UINT offset = 0;
                                        UINT constants = 16;
                                        context->VSSetConstantBuffers1(
                                            1,
                                            1,
                                            resources.view_proj_buffer.GetAddressOf(),
                                            &offset,
                                            &constants
                                        );

                                        context->PSSetConstantBuffers1(
                                            1,
                                            1,
                                            resources.view_proj_buffer.GetAddressOf(),
                                            &offset,
                                            &constants
                                        );
                                    }

                                    // Apply the model constant buffer to the vertex shader.
                                    context->VSSetConstantBuffers1(
                                        0,
                                        1,
                                        resources.particles_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
                                        &resources.particles_resources[rt_idx].vs_constant_buffer_offset,
                                        &resources.particles_resources[rt_idx].vs_constant_buffer_constants
                                    );

                                    context->PSSetConstantBuffers1(
                                        0,
                                        1,
                                        resources.particles_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
                                        &resources.particles_resources[rt_idx].vs_constant_buffer_offset,
                                        &resources.particles_resources[rt_idx].vs_constant_buffer_constants
                                    );

                                    //TODO bind particles structured buffer
                                    context->VSSetShaderResources(
                                        2,
                                        1,
                                        resources.particles_resources[rt_idx].particles_buffer.resource->getShaderResourceView().GetAddressOf());
                                    context->PSSetShaderResources(
                                        2,
                                        1,
                                        resources.particles_resources[rt_idx].particles_buffer.resource->getShaderResourceView().GetAddressOf());

                                    context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
                                    context->IASetIndexBuffer(NULL, (DXGI_FORMAT)0, 0);
                                    context->IASetInputLayout(NULL);

                                    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                                    //TODO
                                    context->DrawInstanced(
                                        resources.particles_resources[rt_idx].particles_cnt * 6, // Vertex count
                                        static_cast<UINT>(frame.m_view_projections.size()),      // Instance count
                                        0,                                                       // Base vertex location
                                        0                                                        // Start instance location
                                    );
                                }
                            }
                        }
                    }
                );
            }

        }
    }
}

#endif // !ParticlesRenderPass_hpp
