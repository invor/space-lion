/// <copyright file="Dx11ResourceManager.hpp">
/// Copyright © 2018 Michael Becher. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Michael Becher</author>

#ifndef Dx11ResourceManager_hpp
#define Dx11ResourceManager_hpp

#include "../BaseResourceManager.hpp"
#include "GenericTextureLayout.hpp"

#include <dxowl/Buffer.hpp>
#include <dxowl/Mesh.hpp>
#include "../MTQueue.hpp"
#include <dxowl/RenderTarget.hpp>
#include <dxowl/ShaderProgram.hpp>
#include <dxowl/Texture2D.hpp>
#include <dxowl/Texture3D.hpp>
#include <dxowl/VertexDescriptor.hpp>

#include <d3d11_4.h>
#include <dwrite.h>
#include <dxgi.h>
#include <d2d1_2.h>
#include <dwrite_3.h>

#include <future>
#include <types.hpp>

namespace EngineCore
{
    namespace Graphics
    {
        struct FontInfo {
            FontInfo()
                : default_font_name(L"Segoe UI"),
                font_size(18.0f),
                foreground{ 0,0,0,1 },
                background{ 1,1,1,1 },
                text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER),
                special_font_ranges({}),
                custom_font_filepaths({})
            {}

            std::wstring                                              default_font_name;
            float                                                     font_size;
            std::array<float, 4>                                      foreground;
            std::array<float, 4>                                      background;
            DWRITE_TEXT_ALIGNMENT                                     text_alignment;
            std::vector<std::tuple<std::wstring, uint32_t, uint32_t>> special_font_ranges;
            std::vector<std::wstring>                                 custom_font_filepaths;
        };

        namespace Dx11
        {

            class ResourceManager : public BaseResourceManager<dxowl::Buffer, dxowl::Mesh, dxowl::ShaderProgram, dxowl::Texture2D, dxowl::Texture3D>
            {
            public:
                typedef dxowl::VertexDescriptor VertexLayout;
                typedef DXGI_FORMAT IndexFormatType;
                typedef D3D_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;

            public:
                ResourceManager();
                ~ResourceManager() = default;

                void init(ID3D11Device4* d3d11_device, ID3D11DeviceContext4* d3d11_device_context, std::vector<std::wstring> const& custom_font_filepaths = {});

                void clearAllResources();

                ID3D11Device4* getD3D11Device() { return m_d3d11_device; }

                ID3D11DeviceContext4* getD3D11DeviceContext() { return m_d3d11_device_context; }

                void executeRenderThreadTasks() {
                    while (!m_renderThread_tasks.empty())
                    {
                        m_renderThread_tasks.pop()();
                    }
                }

    #pragma region Create buffer
                template<typename BufferDataType>
                ResourceID createStructuredBufferAsync(
                    std::string const& name,
                    std::shared_ptr<std::vector<BufferDataType>> const& data);
    #pragma endregion

    #pragma region Create mesh
                //template<
                //    typename VertexContainer,
                //    typename IndexContainer>
                //ResourceID createMeshAsync(
                //    std::string const& name,
                //    std::shared_ptr<std::vector<VertexContainer>> const& vertex_data,
                //    std::shared_ptr<IndexContainer> const& index_data,
                //    std::shared_ptr<VertexDescriptor> const& vertex_layout,
                //    D3D_PRIMITIVE_TOPOLOGY const mesh_type);

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
                VertexLayout convertGenericGltfVertexLayout(GenericVertexLayout vertex_layout, UINT base_input_slot = 0)
                {
                    VertexLayout retval;

                    UINT input_slot = base_input_slot;

                    for (auto attrib : vertex_layout.attributes)
                    {
                        if (attrib.semantic_name == "NORMAL" /* TODO: also check if other attribute porperties match, i.e. 3 GL_FLOAT for the normal*/)
                        {
                            retval.attributes.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "TANGENT" /* TODO: also check if other attribute porperties match, i.e. 3 GL_FLOAT for the normal*/)
                        {
                            retval.attributes.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
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
                            retval.attributes.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0 });
                            ++input_slot;
                        }
                        else if (attrib.semantic_name == "TEXCOORD_1")
                        {
                            //continue;
                            //TODO float colors ?
                            retval.attributes.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, input_slot, input_slot == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0 });
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
                    std::shared_ptr<std::vector<ShaderFilename>> shader_filenames,
                    std::shared_ptr<std::vector<dxowl::VertexDescriptor>> vertex_layout);

                typedef std::tuple<const void*, size_t, dxowl::ShaderProgram::ShaderType> ShaderData;
                ResourceID createShaderProgram(
                    std::string const& name,
                    std::vector<ShaderData> const& shader_bytedata,
                    std::vector<dxowl::VertexDescriptor> const& vertex_layout);

                //std::future<WeakResource<ShaderProgram>> recreateShaderProgram(
                //    ResourceID const& resource_id,
                //    std::shared_ptr<std::pair<std::string, ShaderProgram::ShaderType>> const& shader_filenames,
                //    std::shared_ptr<VertexDescriptor> const& vertex_layout);
    #pragma endregion

    #pragma region Create 2D textures
                constexpr DXGI_FORMAT convertInternalFormat(GenericTextureLayout::InternalFormat internal_format) {
                    DXGI_FORMAT retval = DXGI_FORMAT_UNKNOWN;

                    switch (internal_format)
                    {
                    case GenericTextureLayout::InternalFormat::R8:
                        retval = DXGI_FORMAT_R8_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::R8_SNORM:
                        retval = DXGI_FORMAT_R8_SNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::R16:
                        retval = DXGI_FORMAT_R16_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::R16_SNORM:
                        retval = DXGI_FORMAT_R16_SNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RG8:
                        retval = DXGI_FORMAT_R8G8_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RG8_SNORM:
                        retval = DXGI_FORMAT_R8G8_SNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RG16:
                        retval = DXGI_FORMAT_R16G16_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RG16_SNORM:
                        retval = DXGI_FORMAT_R16G16_SNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::R3_G3_B2:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB4:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB5:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB8:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB8_SNORM:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB10:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB12:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB16_SNORM:
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA2:
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA4:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB5_A1:
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA8:
                        retval = DXGI_FORMAT_R8G8B8A8_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA8_SNORM:
                        retval = DXGI_FORMAT_R8G8B8A8_SNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB10_A2:
                        retval = DXGI_FORMAT_R10G10B10A2_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB10_A2UI:
                        retval = DXGI_FORMAT_R10G10B10A2_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA12:
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA16:
                        retval = DXGI_FORMAT_R16G16B16A16_UNORM;
                        break;
                    case GenericTextureLayout::InternalFormat::SRGB8:
                        break;
                    case GenericTextureLayout::InternalFormat::SRGB8_ALPHA8:
                        break;
                    case GenericTextureLayout::InternalFormat::R16F:
                        retval = DXGI_FORMAT_R16_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG16F:
                        retval = DXGI_FORMAT_R16G16_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB16F:
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA16F:
                        retval = DXGI_FORMAT_R16G16B16A16_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::R32F:
                        retval = DXGI_FORMAT_R32_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG32F:
                        retval = DXGI_FORMAT_R32G32_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB32F:
                        retval = DXGI_FORMAT_R32G32B32_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA32F:
                        retval = DXGI_FORMAT_R32G32B32A32_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::R11F_G11F_B10F:
                        retval = DXGI_FORMAT_R11G11B10_FLOAT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB9_E5:
                        retval = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                        break;
                    case GenericTextureLayout::InternalFormat::R8I:
                        retval = DXGI_FORMAT_R8_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::R8UI:
                        retval = DXGI_FORMAT_R8_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::R16I:
                        retval = DXGI_FORMAT_R16_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::R16UI:
                        retval = DXGI_FORMAT_R16_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::R32I:
                        retval = DXGI_FORMAT_R32_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::R32UI:
                        retval = DXGI_FORMAT_R32_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG8I:
                        retval = DXGI_FORMAT_R8G8_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG8UI:
                        retval = DXGI_FORMAT_R8G8_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG16I:
                        retval = DXGI_FORMAT_R16G16_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG16UI:
                        retval = DXGI_FORMAT_R16G16_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG32I:
                        retval = DXGI_FORMAT_R32G32_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RG32UI:
                        retval = DXGI_FORMAT_R32G32_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB8I:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB8UI:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB16I:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB16UI:
                        break;
                    case GenericTextureLayout::InternalFormat::RGB32I:
                        retval = DXGI_FORMAT_R32G32B32_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGB32UI:
                        retval = DXGI_FORMAT_R32G32B32_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA8I:
                        retval = DXGI_FORMAT_R8G8B8A8_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA8UI:
                        retval = DXGI_FORMAT_R8G8B8A8_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA16I:
                        retval = DXGI_FORMAT_R16G16B16A16_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA16UI:
                        retval = DXGI_FORMAT_R16G16B16A16_UINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA32I:
                        retval = DXGI_FORMAT_R32G32B32A32_SINT;
                        break;
                    case GenericTextureLayout::InternalFormat::RGBA32UI:
                        retval = DXGI_FORMAT_R32G32B32A32_UINT;
                        break;
                    default:
                        break;
                    }

                    return retval;
                }

                D3D11_TEXTURE2D_DESC convertGenericTextureLayout(GenericTextureLayout texture_layout)
                {
                    D3D11_TEXTURE2D_DESC retval;
                    retval.Width = texture_layout.width;
                    retval.Height = texture_layout.height;
                    retval.MipLevels = texture_layout.levels;
                    retval.ArraySize = texture_layout.depth;
                    retval.Format = convertInternalFormat(texture_layout.internal_format);
                    retval.CPUAccessFlags = 0;
                    retval.SampleDesc.Count = 1;
                    retval.SampleDesc.Quality = 0;
                    retval.Usage = D3D11_USAGE_DEFAULT;
                    retval.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
                    retval.CPUAccessFlags = 0;
                    retval.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

                    return retval;
                }

                // BIG TODO: get dx/gl interface synched

                template<
                    typename TexelDataContainer>
                ResourceID createTexture2DAsync(
                    std::string const& name,
                    std::shared_ptr<std::vector<TexelDataContainer>> const& data,
                    D3D11_TEXTURE2D_DESC const&               desc,
                    D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view);

                ResourceID createTexture2DAsync(
                    std::string const& name,
                    D3D11_TEXTURE2D_DESC const& desc,
                    void* data,
                    bool generate_mipmap);

                template<
                    typename TexelDataContainer>
                WeakResource<dxowl::Texture2D> createTexture2D(
                    std::string const&                     name,
                    std::vector<TexelDataContainer> const& data,
                    D3D11_TEXTURE2D_DESC const&               desc,
                    D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view);

                ResourceID createTextTexture2DAsync(
                    std::string const& name,
                    std::wstring const& text,
                    FontInfo const& font_info,
                    D3D11_TEXTURE2D_DESC const& desc,
                    bool generate_mipmap);

                void updateTextTexture2D(
                    ResourceID rsrc_id,
                    std::wstring const& text,
                    FontInfo const& font_info
                );

                void updateTextTexture2DAsync(
                    ResourceID rsrc_id,
                    std::wstring const& text,
                    FontInfo const& font_info
                );

                Vec2 estimateTextTextureSize(
                    FontInfo const& font_info,
                    float dpi,
                    size_t line_length,
                    size_t lines = 1
                );

    #pragma endregion

    #pragma region Create Render Targets

                ResourceID createRenderTargetAsync(
                    std::string const&                     name,
                    D3D11_TEXTURE2D_DESC const&               desc,
                    D3D11_SHADER_RESOURCE_VIEW_DESC const& shdr_rsrc_view,
                    D3D11_RENDER_TARGET_VIEW_DESC const&   rndr_tgt_view_desc);

    #pragma endregion

    #pragma region Access resources

                WeakResource<dxowl::RenderTarget> getRenderTarget(std::string const& name) const;

                WeakResource<dxowl::RenderTarget> getRenderTarget(ResourceID rsrc_id) const;

    #pragma endregion

            private:

                ID3D11Device4* m_d3d11_device;
                ID3D11DeviceContext4* m_d3d11_device_context;

                EngineCore::Utility::MTQueue<std::function<void()>> m_renderThread_tasks;

                std::vector<Resource<dxowl::RenderTarget>> m_render_targets;
                std::unordered_map<unsigned int, size_t>   m_id_to_renderTarget_idx;
                std::unordered_map<std::string, size_t>    m_name_to_renderTarget_idx;

                mutable std::shared_mutex m_renderTargets_mutex;

                struct TextResources {
                    winrt::com_ptr<ID2D1Factory2>           m_d2d_factory;
                    winrt::com_ptr<ID2D1Device1>            m_d2d_device;
                    winrt::com_ptr<ID2D1DeviceContext1>     m_d2d_context;
                    winrt::com_ptr<IDWriteFactory5>         m_dwrite_factory;

                    winrt::com_ptr <IDWriteFontCollection1> m_custom_font_collection;
                };

                std::unique_ptr<TextResources> m_text_resources;

            };

            //template<typename VertexContainer, typename IndexContainer>
            //inline ResourceID ResourceManager::createMeshAsync(std::string const & name, std::shared_ptr<std::vector<VertexContainer>> const & vertex_data, std::shared_ptr<IndexContainer> const & index_data, std::shared_ptr<VertexDescriptor> const & vertex_layout, D3D_PRIMITIVE_TOPOLOGY const mesh_type)
            //{
            //    std::unique_lock<std::shared_mutex> mesh_lock(m_meshes_mutex);
            //
            //    unsigned int idx = m_meshes.size();
            //    ResourceID rsrc_id = generateResourceID();
            //    m_meshes.push_back(Resource<Mesh>(rsrc_id));
            //
            //    m_id_to_mesh_idx.insert(std::pair<unsigned int, unsigned int>(m_meshes.back().id.value(), idx));
            //
            //    //GEngineCore::graphicsBackend().addSingleExecutionGpuTask([this, idx, name, vertex_byte_size, index_byte_size, vertex_description, mesh_type]() {
            //    //    //GEngineCore::renderingPipeline().addSingleExecutionGpuTask([this, idx, name, vertex_byte_size, index_byte_size, vertex_description, mesh_type]() {
            //    //
            //    //    std::unique_lock<std::shared_mutex> mesh_lock(m_mesh_mutex);
            //    //
            //    //    m_meshes[idx].resource = std::make_unique<Mesh>(nullptr, vertex_byte_size, nullptr, index_byte_size, vertex_description, GL_UNSIGNED_INT, GL_STATIC_DRAW, mesh_type);
            //    //    m_meshes[idx].state = READY;
            //    //});
            //
            //    return m_meshes.back().id;
            //}

            template<typename BufferDataType>
            inline ResourceID ResourceManager::createStructuredBufferAsync(
                std::string const& name,
                std::shared_ptr<std::vector<BufferDataType>> const& data)
            {
                std::unique_lock<std::shared_mutex> lock(m_buffers_mutex);

                size_t idx = m_buffers.size();
                ResourceID rsrc_id = generateResourceID();
                m_buffers.push_back(Resource<dxowl::Buffer>(rsrc_id));

                addBufferIndex(rsrc_id.value(), name, idx);

                m_renderThread_tasks.push(
                    [this,idx,data]() {



                        D3D11_BUFFER_DESC desc = { 0 };
                        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                        desc.ByteWidth = sizeof(BufferDataType) * data->size();
                        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                        desc.StructureByteStride = sizeof(BufferDataType);

                        D3D11_SHADER_RESOURCE_VIEW_DESC shdr_rsrc_view_desc = { 0 };
                        shdr_rsrc_view_desc.Format = DXGI_FORMAT_UNKNOWN;
                        shdr_rsrc_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
                        shdr_rsrc_view_desc.Buffer.NumElements = data->size();

                        this->m_buffers[idx].resource = std::make_unique<dxowl::Buffer>(
                            m_d3d11_device,
                            desc,
                            shdr_rsrc_view_desc,
                            *(data));

                        this->m_buffers[idx].state = READY;
                    }
                );

                //auto result = std::async([this, idx, data, desc, shdr_rsrc_view]() {
                //
                //    //this->m_textures_2d[idx].resource = std::make_unique<Texture2D>(
                //    //    m_device_resources->GetD3DDevice(),
                //    //    *data,
                //    //    desc,
                //    //    shdr_rsrc_view);
                //    //
                //    this->m_textures_2d[idx].state = READY;
                //});

                return m_buffers[idx].id;
            }

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

                        std::vector<const void*> data_ptrs;

                        for (auto& dc : *data) {
                            data_ptrs.push_back(dc.data());
                        }

                        this->m_textures_2d[idx].resource = std::make_unique<dxowl::Texture2D>(
                            m_d3d11_device,
                            data_ptrs,
                            desc,
                            shdr_rsrc_view);

                        this->m_textures_2d[idx].state = READY;
                    }
                );

                //auto result = std::async([this, idx, data, desc, shdr_rsrc_view]() {
                //
                //    //this->m_textures_2d[idx].resource = std::make_unique<Texture2D>(
                //    //    m_device_resources->GetD3DDevice(),
                //    //    *data,
                //    //    desc,
                //    //    shdr_rsrc_view);
                //    //
                //    this->m_textures_2d[idx].state = READY;
                //});

                return m_textures_2d[idx].id;
            }

            inline ResourceID ResourceManager::createTexture2DAsync(
                std::string const& name,
                D3D11_TEXTURE2D_DESC const& desc,
                void* data,
                bool generate_mipmap)
            {
                std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);

                size_t idx = m_textures_2d.size();
                ResourceID rsrc_id = generateResourceID();
                m_textures_2d.push_back(Resource<dxowl::Texture2D>(rsrc_id));

                addTextureIndex(rsrc_id.value(), name, idx);

                m_renderThread_tasks.push(
                    [this, idx, data, desc, generate_mipmap]() {

                        std::vector<const void *> data_ptrs = { data };

                        D3D11_SHADER_RESOURCE_VIEW_DESC shdr_rsrc_view;
                        shdr_rsrc_view.Format = desc.Format;
                        shdr_rsrc_view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                        shdr_rsrc_view.TextureCube.MipLevels = desc.MipLevels;
                        shdr_rsrc_view.TextureCube.MostDetailedMip = 0;

                        this->m_textures_2d[idx].resource = std::make_unique<dxowl::Texture2D>(
                            m_d3d11_device,
                            data_ptrs,
                            desc,
                            shdr_rsrc_view,
                            generate_mipmap);

                        this->m_textures_2d[idx].state = READY;
                    }
                );

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

            inline WeakResource<dxowl::RenderTarget> ResourceManager::getRenderTarget(std::string const& name) const
            {
                std::shared_lock<std::shared_mutex> rt_lock(m_renderTargets_mutex);

                auto search = m_name_to_renderTarget_idx.find(name);

                WeakResource<dxowl::RenderTarget> retval(invalidResourceID(), nullptr, NOT_READY);

                if(search != m_name_to_renderTarget_idx.end())
                {
                    retval.id = m_render_targets[search->second].id;
                    retval.resource = m_render_targets[search->second].resource.get();
                    retval.state = m_render_targets[search->second].state;
                }

                return retval;
            }

            inline WeakResource<dxowl::RenderTarget> ResourceManager::getRenderTarget(ResourceID rsrc_id) const
            {
                std::shared_lock<std::shared_mutex> rt_lock(m_renderTargets_mutex);

                auto search = m_id_to_renderTarget_idx.find(rsrc_id.value());

                WeakResource<dxowl::RenderTarget> retval(invalidResourceID(), nullptr, NOT_READY);

                if (search != m_id_to_renderTarget_idx.end())
                {
                    retval.id = m_render_targets[search->second].id;
                    retval.resource = m_render_targets[search->second].resource.get();
                    retval.state = m_render_targets[search->second].state;
                }

                return retval;
            }

        }
    }
}

#endif