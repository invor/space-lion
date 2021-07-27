/// <copyright file="Dx11ResourceManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef Dx11ResourceManager_hpp
#define Dx11ResourceManager_hpp

#include "../BaseResourceManager.hpp"

#include <dxowl/Buffer.hpp>
#include <dxowl/Mesh.hpp>
#include "../MTQueue.hpp"
#include <dxowl/RenderTarget.hpp>
#include <dxowl/ShaderProgram.hpp>
#include <dxowl/Texture2D.hpp>
#include <dxowl/Texture3D.hpp>
#include <dxowl/VertexDescriptor.hpp>

#include <d3d11_4.h>

#include <future>

namespace EngineCore
{
	namespace Graphics
	{
		namespace Dx11
		{

			class ResourceManager : public BaseResourceManager<dxowl::Buffer, dxowl::Mesh, dxowl::ShaderProgram, dxowl::Texture2D, dxowl::Texture3D>
			{
            public:
                typedef dxowl::VertexDescriptor VertexLayout;
                typedef DXGI_FORMAT IndexFormatType;
                typedef D3D_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;

			public:
				ResourceManager(ID3D11Device4* d3d11_device, ID3D11DeviceContext4* d3d11_device_context);
				~ResourceManager() = default;
	
				void clearAllResources();

				ID3D11Device4* getD3D11Device() { return m_d3d11_device; }

				ID3D11DeviceContext4* getD3D11DeviceContext() { return m_d3d11_device_context; }

				void executeRenderThreadTasks() {
					while (!m_renderThread_tasks.empty())
					{
						m_renderThread_tasks.pop()();
					}
				}

	#pragma region Create mesh
				//template<
				//	typename VertexContainer,
				//	typename IndexContainer>
				//ResourceID createMeshAsync(
				//	std::string const& name,
				//	std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
				//	std::shared_ptr<IndexContainer> const& index_data,
				//	std::shared_ptr<VertexDescriptor> const& vertex_layout,
				//	D3D_PRIMITIVE_TOPOLOGY const mesh_type);

                constexpr size_t computeIndexByteSize(IndexFormatType index_type)
                {
                    return dxowl::computeByteSize(index_type);
                }

                size_t computeVertexByteSize(dxowl::VertexDescriptor vertex_layout) const
                {
                    size_t retval = 0;

                    for (auto& attrib : vertex_layout.attributes)
                    {
                        retval += dxowl::computeAttributeByteSize(attrib);
                    }

                    return retval;
                }

                size_t computeVertexByteSize(std::vector<dxowl::VertexDescriptor> vertex_layout) const
                {
                    size_t retval = 0;
                    for (auto const& vl : vertex_layout) {
                        retval += computeVertexByteSize(vl);
                    }
                    return retval;
                }

                constexpr IndexFormatType convertGenericIndexType(uint32_t index_type)
                {
                    IndexFormatType retval = DXGI_FORMAT_UNKNOWN;

                    switch (index_type)
                    {
                    case 5120:
                        retval = DXGI_FORMAT_R8_SINT;
                        break;
                    case 5121:
                        retval = DXGI_FORMAT_R8_UINT;
                        break;
                    case 5122:
                        retval = DXGI_FORMAT_R16_SINT;
                        break;
                    case 5123:
                        retval = DXGI_FORMAT_R16_UINT;
                        break;
                    case 5125:
                        retval = DXGI_FORMAT_R32_UINT;
                        break;
                    default:
                        //??
                        break;
                    }

                    return retval;
                }

                constexpr PrimitiveTopologyType convertGenericPrimitiveTopology(uint32_t primitive_type)
                {
                    PrimitiveTopologyType retval = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

                    switch (primitive_type)
                    {
                    case 0x0000:
                        retval = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                        break;
                    case 0x0001:
                        retval = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                        break;
                    case 0x0002:
                        retval = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                        break;
                    case 0x0003:
                        retval = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                        break;
                    case 0x0004:
                        retval = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                        break;
                    case 0x0005:
                        retval = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                        break;
                    case 0x0006:
                        retval = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                        break;
                    default:
                        break;
                    }

                    return retval;
                }

                /**
                 * Create a DirectX vertex description from the generic vertex layout struct, assuming it contains an
                 * OpenGL-sytle layout description, because is was created from a gltf file
                 */
                VertexLayout convertGenericGltfVertexLayout(GenericVertexLayout vertex_layout)
                {
                    VertexLayout retval;

                    UINT input_slot = 0;

                    for (auto attrib : vertex_layout.attributes)
                    {
                        if (attrib.semantic_name == "NORMAL" /* TODO: also check if other attribute porperties match, i.e. 3 GL_FLOAT for the normal*/)
                        {
                            retval.attributes.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "POSITION")
                        {
                            retval.attributes.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "COLOR_0")
                        {
                            //TODO float colors ?
                            retval.attributes.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "TEXCOORD_0")
                        {
                            //continue;
                            //TODO float colors ?
                            retval.attributes.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "TEXCOORD_1")
                        {
                            //continue;
                            //TODO float colors ?
                            retval.attributes.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                    }

					retval.stride = vertex_layout.stride;

                    return retval;
                }

                ResourceID allocateMeshAsync(
                    std::string const& name,
                    size_t vertex_cnt,
                    size_t index_cnt,
					std::shared_ptr<std::vector<VertexLayout>> const& vertex_layout,
					DXGI_FORMAT const index_type,
					D3D_PRIMITIVE_TOPOLOGY const mesh_type);

				template<
					typename VertexContainer,
					typename IndexContainer>
				void updateMesh(
					ResourceID rsrc_id,
					size_t vertex_offset,
					size_t index_offset,
					std::vector<VertexContainer> const& vertex_data,
					IndexContainer const& index_data);

				template<
					typename VertexContainer,
					typename IndexContainer>
				void updateMeshAsync(
					ResourceID rsrc_id,
					size_t vertex_offset,
					size_t index_offset,
					std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
					std::shared_ptr<IndexContainer> const& index_data);

	#pragma endregion
	
	#pragma region Create shader program
				typedef std::pair<std::string, dxowl::ShaderProgram::ShaderType> ShaderFilename;
                ResourceID createShaderProgramAsync(
					std::string const& name,
					std::shared_ptr<std::vector<ShaderFilename>> const& shader_filenames,
					std::shared_ptr<std::vector<dxowl::VertexDescriptor>> const& vertex_layout);

				typedef std::tuple<const void*, size_t, dxowl::ShaderProgram::ShaderType> ShaderData;
				ResourceID createShaderProgram(
					std::string const& name,
					std::vector<ShaderData> const& shader_bytedata,
					std::vector<dxowl::VertexDescriptor> const& vertex_layout);
	
				//std::future<WeakResource<ShaderProgram>> recreateShaderProgram(
				//	ResourceID const& resource_id,
				//	std::shared_ptr<std::pair<std::string, ShaderProgram::ShaderType>> const& shader_filenames,
				//	std::shared_ptr<VertexDescriptor> const& vertex_layout);
	#pragma endregion
	
	#pragma region Create 2D textures
				template<
					typename TexelDataContainer>
					ResourceID createTexture2DAsync(
						std::string const& name,
						std::shared_ptr<std::vector<TexelDataContainer>> const& data,
						D3D11_TEXTURE2D_DESC const&	           desc,
						D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view);
	
				template<
					typename TexelDataContainer>
					WeakResource<dxowl::Texture2D> createTexture2D(
						std::string const&                     name,
						std::vector<TexelDataContainer> const& data,
						D3D11_TEXTURE2D_DESC const&	           desc,
						D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view);
	#pragma endregion

	#pragma region Create Render Targets

				ResourceID createRenderTargetAsync(
					std::string const&                     name,
					D3D11_TEXTURE2D_DESC const&	           desc,
					D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view,
					D3D11_RENDER_TARGET_VIEW_DESC const&   rndr_tgt_view_desc);

	#pragma endregion

			private:

				ID3D11Device4* m_d3d11_device;
				ID3D11DeviceContext4* m_d3d11_device_context;

				EngineCore::Utility::MTQueue<std::function<void()>> m_renderThread_tasks;

				std::vector<Resource<dxowl::RenderTarget>> m_render_targets;
				std::unordered_map<unsigned int, size_t>   m_id_to_renderTarget_idx;
				std::unordered_map<std::string, size_t>    m_name_to_renderTarget_idx;

				mutable std::shared_mutex m_renderTargets_mutex;
			
			};

			//template<typename VertexContainer, typename IndexContainer>
			//inline ResourceID ResourceManager::createMeshAsync(std::string const & name, std::shared_ptr<std::vector<VertexContainer>> const & vertex_data, std::shared_ptr<IndexContainer> const & index_data, std::shared_ptr<VertexDescriptor> const & vertex_layout, D3D_PRIMITIVE_TOPOLOGY const mesh_type)
			//{
			//	std::unique_lock<std::shared_mutex> mesh_lock(m_meshes_mutex);
			//
			//	unsigned int idx = m_meshes.size();
			//	ResourceID rsrc_id = generateResourceID();
			//	m_meshes.push_back(Resource<Mesh>(rsrc_id));
			//
			//	m_id_to_mesh_idx.insert(std::pair<unsigned int, unsigned int>(m_meshes.back().id.value(), idx));
			//
			//	//GEngineCore::graphicsBackend().addSingleExecutionGpuTask([this, idx, name, vertex_byte_size, index_byte_size, vertex_description, mesh_type]() {
			//	//	//GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, name, vertex_byte_size, index_byte_size, vertex_description, mesh_type]() {
			//	//
			//	//	std::unique_lock<std::shared_mutex> mesh_lock(m_mesh_mutex);
			//	//
			//	//	m_meshes[idx].resource = std::make_unique<Mesh>(nullptr, vertex_byte_size, nullptr, index_byte_size, vertex_description, GL_UNSIGNED_INT, GL_STATIC_DRAW, mesh_type);
			//	//	m_meshes[idx].state = READY;
			//	//});
			//
			//	return m_meshes.back().id;
			//}

			template<typename VertexContainer, typename IndexContainer>
			inline void ResourceManager::updateMesh(
				ResourceID rsrc_id,
				size_t vertex_offset,
				size_t index_offset,
				std::vector<VertexContainer> const & vertex_data,
				IndexContainer const & index_data)
			{
				std::shared_lock<std::shared_mutex> lock(m_meshes_mutex);

				auto query = m_id_to_mesh_idx.find(rsrc_id.value());

				if (query != m_id_to_mesh_idx.end())
				{
					std::vector<dxowl::VertexDescriptor> vertex_layout = m_meshes[query->second].resource->getVertexLayout();

					//TODO some sanity checks, such as attrib cnt and mesh size?

					for (size_t vb_idx = 0; vb_idx < vertex_data.size(); ++vb_idx) {
						size_t attribs_byte_size = computeVertexByteSize(vertex_layout[vb_idx]);
						size_t vertex_buffer_byte_offset = attribs_byte_size * vertex_offset;
						m_meshes[query->second].resource->loadVertexSubData(
							m_d3d11_device_context,
							vb_idx,
							vertex_buffer_byte_offset,
							vertex_data[vb_idx]);
					}

					size_t index_type_byte_size = dxowl::computeByteSize(m_meshes[query->second].resource->getIndexFormat());
					size_t index_byte_offset = index_offset * index_type_byte_size;
					m_meshes[query->second].resource->loadIndexSubdata(m_d3d11_device_context, index_byte_offset, index_data);
				}
			}

			template<typename VertexContainer, typename IndexContainer>
			inline void ResourceManager::updateMeshAsync(
				ResourceID rsrc_id,
				size_t vertex_offset,
				size_t index_offset,
				std::shared_ptr<std::vector<VertexContainer>> const & vertex_data,
				std::shared_ptr<IndexContainer> const & index_data)
			{
				m_renderThread_tasks.push(
				/*std::async(*/[this, rsrc_id, vertex_offset, index_offset, vertex_data, index_data]() {

					std::shared_lock<std::shared_mutex> lock(m_meshes_mutex);

					auto query = m_id_to_mesh_idx.find(rsrc_id.value());

					if (query != m_id_to_mesh_idx.end())
					{
						while (!(m_meshes[query->second].state == READY)) {} // TODO somehow do this more efficiently

						std::vector<dxowl::VertexDescriptor> vertex_layout = m_meshes[query->second].resource->getVertexLayout();

						//TODO some sanity checks, such as attrib cnt and mesh size?

						for (size_t vb_idx = 0; vb_idx < vertex_data->size(); ++vb_idx) {
							size_t attribs_byte_size = computeVertexByteSize(vertex_layout[vb_idx]);
							size_t vertex_buffer_byte_offset = attribs_byte_size * vertex_offset;
							m_meshes[query->second].resource->loadVertexSubData(
								m_d3d11_device_context,
								vb_idx,
								vertex_buffer_byte_offset,
								(*vertex_data)[vb_idx]);
						}

						size_t index_type_byte_size = dxowl::computeByteSize(m_meshes[query->second].resource->getIndexFormat());
						size_t index_byte_offset = index_offset * index_type_byte_size;
						m_meshes[query->second].resource->loadIndexSubdata(
							m_d3d11_device_context, 
							index_byte_offset, 
							*index_data);
					}

				});
			}

			template<typename TexelDataContainer>
			inline ResourceID ResourceManager::createTexture2DAsync(
				std::string const &                                      name, 
				std::shared_ptr<std::vector<TexelDataContainer>> const & data,
				D3D11_TEXTURE2D_DESC const &                             desc,
				D3D11_SHADER_RESOURCE_VIEW_DESC const &                  shdr_rsrc_view)
			{
				std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);

				size_t idx = m_textures_2d.size();
				ResourceID rsrc_id = generateResourceID();
				m_textures_2d.push_back(Resource<dxowl::Texture2D>(rsrc_id));

				addTextureIndex(rsrc_id.value(), name, idx);

				m_renderThread_tasks.push(
					[this, idx, data, desc, shdr_rsrc_view]() {
					
						this->m_textures_2d[idx].resource = std::make_unique<dxowl::Texture2D>(
							m_d3d11_device,
							*data,
							desc,
							shdr_rsrc_view);
						
						this->m_textures_2d[idx].state = READY;
					}
				);

				//auto result = std::async([this, idx, data, desc, shdr_rsrc_view]() {
				//
				//	//this->m_textures_2d[idx].resource = std::make_unique<Texture2D>(
				//	//	m_device_resources->GetD3DDevice(),
				//	//	*data,
				//	//	desc,
				//	//	shdr_rsrc_view);
				//	//
				//	this->m_textures_2d[idx].state = READY;
				//});

				return m_textures_2d[idx].id;
			}

			template<typename TexelDataContainer>
			inline WeakResource<dxowl::Texture2D> ResourceManager::createTexture2D(
				std::string const & name, 
				std::vector<TexelDataContainer> const & data,
				D3D11_TEXTURE2D_DESC const & desc, 
				D3D11_SHADER_RESOURCE_VIEW_DESC const & shdr_rsrc_view)
			{
				return WeakResource<dxowl::Texture2D>();
			}

			inline ResourceID ResourceManager::createRenderTargetAsync(
				std::string const & name, 
				D3D11_TEXTURE2D_DESC const & desc, 
				D3D11_SHADER_RESOURCE_VIEW_DESC const & shdr_rsrc_view,
				D3D11_RENDER_TARGET_VIEW_DESC const & rndr_tgt_view_desc)
			{
				std::unique_lock<std::shared_mutex> lock(m_renderTargets_mutex);

				size_t idx = m_render_targets.size();
				ResourceID rsrc_id = generateResourceID();
				m_render_targets.push_back(Resource<dxowl::RenderTarget>(rsrc_id));

				m_id_to_renderTarget_idx.insert(std::pair<unsigned int, size_t>(rsrc_id.value(), idx));
				m_name_to_renderTarget_idx.insert(std::pair<std::string, size_t>(name, idx));

				std::async([this, idx, desc, shdr_rsrc_view, rndr_tgt_view_desc]() {

					this->m_render_targets[idx].resource = std::make_unique<dxowl::RenderTarget>(
						m_d3d11_device,
						desc,
						shdr_rsrc_view,
						rndr_tgt_view_desc);

					this->m_render_targets[idx].state = READY;
				});

				return m_render_targets[idx].id;
			}

		}
	}
}

#endif