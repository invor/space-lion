#include "SimpleForwardRenderingPipeline.hpp"

#include "../Dx11/Dx11Frame.hpp"
#include "CameraComponent.hpp"
#include "MaterialComponentManager.hpp"
#include "MeshComponentManager.hpp"
#include "RenderTaskComponentManager.hpp"
#include "ResourceManager.hpp"
#include "TransformComponentManager.hpp"
#include "../WorldState.hpp"

#include <dxowl/Buffer.hpp>
#include <dxowl/Mesh.hpp>
#include <dxowl/ShaderProgram.hpp>

void EngineCore::Graphics::Dx11::setupSimpleForwardRenderingPipeline(
    EngineCore::Graphics::Dx11::Frame & frame,
    WorldState& world,
    ResourceManager& resource_mngr)
{
	struct GeomPassData
	{
		struct ViewProjectionConstantBuffer {
			Mat4x4 view_projection;
			Mat4x4 view_inverse;
		};

		struct VSConstantBuffer
		{
			Mat4x4 transform;

			Mat4x4 normal_matrix;

			Vec4   albedo_colour;
			Vec4   specular_colour;
			float  roughness;
			Vec3   padding10;
			Vec4   padding11;

			Mat4x4 padding2;
		};

		struct RenderTaskData
		{
			ResourceID   mesh_resource;
			unsigned int indices_cnt;
			unsigned int first_index;
			unsigned int base_vertex;

			ResourceID   shader_resource;
		};

		ViewProjectionConstantBuffer view_proj_buffer;

		std::vector<VSConstantBuffer> vs_constant_buffer;

		std::vector<RenderTaskData> rt_data;

		size_t opaque_objs_cnt;
		size_t transparent_objs_cnt;
	};

	struct GeomPassResources
	{
		struct BatchResources
		{
			WeakResource<dxowl::ShaderProgram>   shader_prgm;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vs_constant_buffer;
			UINT                                 vs_constant_buffer_offset;
			UINT                                 vs_constant_buffer_constants;
			WeakResource<dxowl::Mesh>            mesh;

			unsigned int indices_cnt;
			unsigned int first_index;
			unsigned int base_vertex;
		};

		ID3D11Device4* d3d11_device;
		ID3D11DeviceContext4* d3d11_device_context;
		Microsoft::WRL::ComPtr<ID3D11Buffer> view_proj_buffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;

		std::shared_ptr<WeakResource<dxowl::Texture2D>> irradiance_map; //TODO fix this hack..

		std::vector<BatchResources> rt_resources;

		std::vector<BatchResources> transparency_rt_resources;
	};

	frame.addRenderPass<GeomPassData, GeomPassResources>("GeomPass",
		[&frame, &world,&resource_mngr](GeomPassData& data, GeomPassResources& resources)
	{
		// obtain access to device resources
		resources.d3d11_device = resource_mngr.getD3D11Device();
		resources.d3d11_device_context = resource_mngr.getD3D11DeviceContext();

		auto& cam_mngr = world.get<CameraComponentManager>();
		auto& mesh_mngr = world.get<MeshComponentManager<ResourceManager>>();
		auto& mtl_mngr = world.get<MaterialComponentManager<ResourceManager>>();
		auto& transform_mngr = world.get<EngineCore::Common::TransformComponentManager>();
		auto& renderTask_mngr = world.get<RenderTaskComponentManager<RenderTaskTags::StaticMesh>>();

		// set camera matrices
		Entity camera_entity = cam_mngr.getActiveCamera();
		auto camera_idx = cam_mngr.getIndex(camera_entity).front();
		auto camera_transform_idx = transform_mngr.getIndex(camera_entity);

		data.view_proj_buffer.view_inverse = glm::transpose(transform_mngr.getWorldTransformation(camera_transform_idx));

		if (frame.m_window_width != 0 && frame.m_window_height != 0) {
			cam_mngr.setAspectRatio(camera_idx, static_cast<float>(frame.m_window_width) / static_cast<float>(frame.m_window_height));
			cam_mngr.updateProjectionMatrix(camera_idx);
			data.view_proj_buffer.view_projection = glm::transpose(cam_mngr.getProjectionMatrix(camera_idx) * glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx)));
		}

		auto rts = renderTask_mngr.getComponentDataCopy();

		data.vs_constant_buffer.reserve(rts.size());
		data.rt_data.reserve(rts.size());

		data.opaque_objs_cnt = 0;
		data.transparent_objs_cnt = 0;

		for (auto const& rt : rts)
		{
			data.vs_constant_buffer.push_back(GeomPassData::VSConstantBuffer());

			auto mtl_idx = mtl_mngr.getIndex(rt.entity);
			if (!mtl_idx.empty())
			{
				auto albedo_colour = mtl_mngr.getAlbedoColour(mtl_idx[rt.mtl_component_subidx]);
				auto specular_colour = mtl_mngr.getSpecularColour(mtl_idx[rt.mtl_component_subidx]);
				auto roughness = mtl_mngr.getRoughness(mtl_idx[rt.mtl_component_subidx]);

				data.vs_constant_buffer.back().albedo_colour = Vec4(albedo_colour[0], albedo_colour[1], albedo_colour[2], albedo_colour[3]);
				data.vs_constant_buffer.back().specular_colour = Vec4(specular_colour[0], specular_colour[1], specular_colour[2], specular_colour[3]);
				data.vs_constant_buffer.back().roughness = roughness;

				if (albedo_colour[3] > 0.999f){
					data.opaque_objs_cnt += 1;
				}
				else{
					data.transparent_objs_cnt += 1;
				}
			}
			else
			{
				data.vs_constant_buffer.back().albedo_colour = Vec4(1.0f, 0.0f, 1.0f, 1.0f);
				data.vs_constant_buffer.back().specular_colour = Vec4(0.04f, 0.04f, 0.04f, 1.0f);
				data.vs_constant_buffer.back().roughness = 1.0f;

				data.opaque_objs_cnt += 1;
			}

			auto transform_idx = transform_mngr.getIndex(rt.entity);
			if(!transform_idx < (std::numeric_limits<size_t>::max)())
			{
				data.vs_constant_buffer.back().transform = glm::transpose(transform_mngr.getWorldTransformation(transform_idx));
				data.vs_constant_buffer.back().normal_matrix = glm::transpose(glm::inverse(transform_mngr.getWorldTransformation(transform_idx)));
			}

			auto mesh_comp_idx = mesh_mngr.getIndex(rt.entity);
			auto draw_params = mesh_mngr.getDrawIndexedParams(mesh_comp_idx[rt.mesh_component_subidx]);
			data.rt_data.push_back(
				{
					rt.mesh,
					std::get<0>(draw_params),
					std::get<1>(draw_params),
					std::get<2>(draw_params),
					rt.shader_prgm
				}
			);
		}
	},
		[&world,&resource_mngr](GeomPassData& data, GeomPassResources& resources)
	{
        resource_mngr.executeRenderThreadTasks();

		// Create sampler state. TODO use resource management for persistent pipeline states
		{
			D3D11_SAMPLER_DESC tex_sampler_desc;
			tex_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			tex_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			tex_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			tex_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			tex_sampler_desc.MipLODBias = 0.0f;
			tex_sampler_desc.MaxAnisotropy = 1;
			tex_sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			tex_sampler_desc.BorderColor[0] = 0;
			tex_sampler_desc.BorderColor[1] = 0;
			tex_sampler_desc.BorderColor[2] = 0;
			tex_sampler_desc.BorderColor[3] = 0;
			tex_sampler_desc.MinLOD = 0;
			tex_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

			// Create the texture sampler state.
			HRESULT result = resources.d3d11_device->CreateSamplerState(&tex_sampler_desc, &(resources.sampler_state));
		}

		// TODO change to proper name...
		resources.irradiance_map = std::make_unique<WeakResource<dxowl::Texture2D>>(resource_mngr.getTexture2DResource("debug_cubemap"));

		//TODO create constant buffers
		{
			D3D11_SUBRESOURCE_DATA constantBufferData = { 0 };
			constantBufferData.pSysMem = &(data.view_proj_buffer);
			constantBufferData.SysMemPitch = 0;
			constantBufferData.SysMemSlicePitch = 0;
			const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(GeomPassData::ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			winrt::check_hresult(
				resources.d3d11_device->CreateBuffer(
					&constantBufferDesc,
					&constantBufferData,
					&resources.view_proj_buffer
				));
		}

		Microsoft::WRL::ComPtr<ID3D11Buffer> vs_constant_buffer(nullptr);

		if (data.vs_constant_buffer.size() > 0)
		{
			D3D11_SUBRESOURCE_DATA constantBufferData = { 0 };
			constantBufferData.pSysMem = data.vs_constant_buffer.data();
			constantBufferData.SysMemPitch = 0;
			constantBufferData.SysMemSlicePitch = 0;
			const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(GeomPassData::VSConstantBuffer)*data.vs_constant_buffer.size(), D3D11_BIND_CONSTANT_BUFFER);
			winrt::check_hresult(
				resources.d3d11_device->CreateBuffer(
					&constantBufferDesc,
					&constantBufferData,
					&vs_constant_buffer
				));
		}
		
		resources.rt_resources.reserve(data.opaque_objs_cnt);
		resources.transparency_rt_resources.reserve(data.transparent_objs_cnt);

		for (size_t rt_idx = 0; rt_idx < data.rt_data.size(); ++rt_idx)
		{
			if (data.vs_constant_buffer[rt_idx].albedo_colour.w > 0.99f) // opaque obj
			{
				WeakResource<dxowl::ShaderProgram>	shader_prgm = resource_mngr.getShaderProgramResource(data.rt_data[rt_idx].shader_resource);

				WeakResource<dxowl::Mesh> mesh = resource_mngr.getMeshResource(data.rt_data[rt_idx].mesh_resource);;

				resources.rt_resources.push_back({ shader_prgm ,vs_constant_buffer, static_cast<UINT>(rt_idx) * 16, 16, mesh, data.rt_data[rt_idx].indices_cnt, data.rt_data[rt_idx].first_index, data.rt_data[rt_idx].base_vertex });
			}
			else  // transparent obj
			{
				WeakResource<dxowl::ShaderProgram>	shader_prgm = resource_mngr.getShaderProgramResource(data.rt_data[rt_idx].shader_resource);

				WeakResource<dxowl::Mesh> mesh = resource_mngr.getMeshResource(data.rt_data[rt_idx].mesh_resource);;

				resources.transparency_rt_resources.push_back({ shader_prgm ,vs_constant_buffer, static_cast<UINT>(rt_idx) * 16, 16, mesh, data.rt_data[rt_idx].indices_cnt, data.rt_data[rt_idx].first_index, data.rt_data[rt_idx].base_vertex });
			}
		}
	},
		[&frame = std::as_const(frame), &resource_mngr](GeomPassData const& data, GeomPassResources const& resources)
	{
		// obtain context for rendering
		const auto device_context = resources.d3d11_device_context;
		const auto device = resources.d3d11_device;

		// clear render target here
		{
			CD3D11_VIEWPORT viewport(
				0.0, 0.0, (float)frame.m_window_width, (float)frame.m_window_height);
			device_context->RSSetViewports(1, &viewport);

			auto render_target_view = frame.m_render_target_view;
			auto depth_stencil_view = frame.m_depth_stencil_view;

			const float clear_color[4] = { 1.0f, 0.2f, 0.4f, 1.0f };
			const float clear_depth = 1.0f;

			// Clear swapchain and depth buffer. NOTE: This will clear the entire render target view, not just the specified view.
			device_context->ClearRenderTargetView(render_target_view, clear_color);
			device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clear_depth, 0);
			//device_context->OMSetDepthStencilState(reversedZ ? m_reversedZDepthNoStencilTest.get() : nullptr, 0);

			ID3D11RenderTargetView* renderTargets[] = { render_target_view };
			device_context->OMSetRenderTargets((UINT)std::size(renderTargets), renderTargets, depth_stencil_view);
		}

		//TODO raise depth buffer resoltion
		D3D11_RASTERIZER_DESC desc;
		
		::ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_SOLID;
		//desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
		device->CreateRasterizerState(&desc, &rasterizer_state);
		
		device_context->RSSetState(rasterizer_state.Get());

		device_context->OMSetDepthStencilState(nullptr, 0);

		device_context->OMSetBlendState(nullptr, nullptr, UINT_MAX);

		// Set shader texture resource and sampler state in the pixel shader.

		if (resources.irradiance_map->state == READY)
		{
			device_context->PSSetSamplers(0, 1, resources.sampler_state.GetAddressOf());
			device_context->PSSetShaderResources(0, 1, resources.irradiance_map->resource->getShaderResourceView().GetAddressOf());
		}

		ResourceID current_mesh = resource_mngr.invalidResourceID();
		ResourceID current_shader = resource_mngr.invalidResourceID();

		// loop over all opaque render tasks
		{
			size_t rt_cnt = resources.rt_resources.size();
			for (size_t rt_idx = 0; rt_idx < rt_cnt; ++rt_idx)
			{
				if(resources.rt_resources[rt_idx].mesh.state == READY
					&& resources.rt_resources[rt_idx].shader_prgm.state == READY)
				{
					dxowl::Mesh* mesh = resources.rt_resources[rt_idx].mesh.resource;
					dxowl::ShaderProgram* shader = resources.rt_resources[rt_idx].shader_prgm.resource;

					if (current_mesh.value() != resources.rt_resources[rt_idx].mesh.id.value())
					{
						current_mesh = resources.rt_resources[rt_idx].mesh.id;

						mesh->setVertexBuffers(device_context, 0);
						mesh->setIndexBuffer(device_context, 0);

						device_context->IASetPrimitiveTopology(mesh->getPrimitiveTopology());
					}

					if (current_shader.value() != resources.rt_resources[rt_idx].shader_prgm.id.value())
					{
						current_shader = resources.rt_resources[rt_idx].shader_prgm.id;

						shader->setInputLayout(device_context);

						shader->setVertexShader(device_context);

						shader->setGeometryShader(device_context);

						shader->setPixelShader(device_context);

						// Apply the view projection constant buffer to shaders
						UINT offset = 0;
						UINT constants = 16;
						device_context->VSSetConstantBuffers1(
							1,
							1,
							resources.view_proj_buffer.GetAddressOf(),
							&offset,
							&constants
						);

						device_context->PSSetConstantBuffers1(
							1,
							1,
							resources.view_proj_buffer.GetAddressOf(),
							&offset,
							&constants
						);
					}
					
					// Apply the model constant buffer to the vertex shader.
					device_context->VSSetConstantBuffers1(
						0,
						1,
						resources.rt_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
						&resources.rt_resources[rt_idx].vs_constant_buffer_offset,
						&resources.rt_resources[rt_idx].vs_constant_buffer_constants
					);

					device_context->PSSetConstantBuffers1(
						0,
						1,
						resources.rt_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
						&resources.rt_resources[rt_idx].vs_constant_buffer_offset,
						&resources.rt_resources[rt_idx].vs_constant_buffer_constants
					);

					// Draw the objects.
					device_context->DrawIndexed(resources.rt_resources[rt_idx].indices_cnt,   // Index count per instance.
						resources.rt_resources[rt_idx].first_index,	// Start index location.
						resources.rt_resources[rt_idx].base_vertex	// Base vertex location.)
					);
					//context->DrawIndexedInstanced(
					//	resources.rt_resources[rt_idx].indices_cnt,   // Index count per instance.
					//	1,									// Instance count.
					//	resources.rt_resources[rt_idx].first_index,	// Start index location.
					//	resources.rt_resources[rt_idx].base_vertex,	// Base vertex location.
					//	0									// Start instance location.
					//);
				}
			}
		}

		// Create add-blend-one blending state for transparent objects.
		{
			Microsoft::WRL::ComPtr<ID3D11BlendState> transparency_blend_state;

			D3D11_BLEND_DESC desc;

			::ZeroMemory(&desc, sizeof(desc));

			//for (size_t i = 0; i < STEREO_BUFFERS; ++i) {
			for (size_t i = 0; i < 2; ++i) {
				desc.RenderTarget[i].BlendEnable = TRUE;
				desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
				desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
				desc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
				desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
				desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
				desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			}

			device->CreateBlendState(&desc, transparency_blend_state.GetAddressOf());

			device_context->OMSetBlendState(transparency_blend_state.Get(), nullptr, UINT_MAX);
		}

		// loop over transparent opaque render tasks
		{
			size_t rt_cnt = resources.transparency_rt_resources.size();
			for (size_t rt_idx = 0; rt_idx < rt_cnt; ++rt_idx)
			{
				if (resources.transparency_rt_resources[rt_idx].mesh.state == READY
					&& resources.transparency_rt_resources[rt_idx].shader_prgm.state == READY)
				{
					dxowl::Mesh* mesh = resources.transparency_rt_resources[rt_idx].mesh.resource;
					dxowl::ShaderProgram* shader = resources.transparency_rt_resources[rt_idx].shader_prgm.resource;

					if (current_mesh.value() != resources.transparency_rt_resources[rt_idx].mesh.id.value())
					{
						current_mesh = resources.transparency_rt_resources[rt_idx].mesh.id;

						mesh->setVertexBuffers(device_context, 0);
						mesh->setIndexBuffer(device_context, 0);

						device_context->IASetPrimitiveTopology(mesh->getPrimitiveTopology());
					}

					if (current_shader.value() != resources.transparency_rt_resources[rt_idx].shader_prgm.id.value())
					{
						current_shader = resources.transparency_rt_resources[rt_idx].shader_prgm.id;

						shader->setInputLayout(device_context);

						shader->setVertexShader(device_context);

						shader->setGeometryShader(device_context);

						shader->setPixelShader(device_context);
					}

					// Apply the model constant buffer to the vertex shader.
					device_context->VSSetConstantBuffers1(
						0,
						1,
						resources.transparency_rt_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
						&resources.transparency_rt_resources[rt_idx].vs_constant_buffer_offset,
						&resources.transparency_rt_resources[rt_idx].vs_constant_buffer_constants
					);

					device_context->PSSetConstantBuffers1(
						0,
						1,
						resources.transparency_rt_resources[rt_idx].vs_constant_buffer.GetAddressOf(),
						&resources.transparency_rt_resources[rt_idx].vs_constant_buffer_offset,
						&resources.transparency_rt_resources[rt_idx].vs_constant_buffer_constants
					);

					// Draw the objects.
					device_context->DrawIndexedInstanced(
						resources.transparency_rt_resources[rt_idx].indices_cnt,   // Index count per instance.
						2,									// Instance count.
						resources.transparency_rt_resources[rt_idx].first_index,	// Start index location.
						resources.transparency_rt_resources[rt_idx].base_vertex,	// Base vertex location.
						0									// Start instance location.
					);
				}
			}
		}

		// clear geometry shader to not confuse other renderings
		device_context->GSSetShader(
			nullptr,
			nullptr,
			0
		);
	}
	);
}
