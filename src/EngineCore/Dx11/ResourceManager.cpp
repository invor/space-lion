#include "ResourceManager.hpp"

#include <winrt/Windows.Storage.h>

#include <iostream>
#include <codecvt>
#include <filesystem>
#include <fstream>

#include "ResourceLoading.hpp"

using namespace EngineCore::Graphics;
using namespace EngineCore::Graphics::Dx11;

namespace {
    // Function that reads from a binary file asynchronously.
    //inline std::future<std::vector<byte>> ReadDataAsync(const std::wstring_view& filename)
    //{
    //    using namespace winrt::Windows::Storage;
    //    using namespace winrt::Windows::Storage::Streams;
    //
    //    IBuffer fileBuffer = co_await PathIO::ReadBufferAsync(filename);
    //
    //    std::vector<byte> returnBuffer;
    //    returnBuffer.resize(fileBuffer.Length());
    //    DataReader::FromBuffer(fileBuffer).ReadBytes(winrt::array_view<uint8_t>(returnBuffer));
    //    co_return returnBuffer;
    //}

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    // See https://github.com/microsoft/OpenXR-MixedReality
    inline std::wstring utf8_to_wide(std::string_view utf8Text) {
        if (utf8Text.empty()) {
            return {};
        }

        std::wstring wideText;
        const int wideLength = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), nullptr, 0);
        if (wideLength == 0) {
            std::cerr<<"utf8_to_wide get size error.";
            return {};
        }

        // MultiByteToWideChar returns number of chars of the input buffer, regardless of null terminitor
        wideText.resize(wideLength, 0);
        const int length = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), wideText.data(), wideLength);
        if (length != wideLength) {
            std::cerr<<"utf8_to_wide convert string error.";
            return {};
        }

        return wideText;
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    // See https://github.com/microsoft/OpenXR-MixedReality
    inline std::string wide_to_utf8(std::wstring_view wideText) {
        if (wideText.empty()) {
            return {};
        }

        std::string narrowText;
        int narrowLength = ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), nullptr, 0, nullptr, nullptr);
        if (narrowLength == 0) {
            std::cerr<<"wide_to_utf8 get size error.";
            return {};
        }

        // WideCharToMultiByte returns number of chars of the input buffer, regardless of null terminitor
        narrowText.resize(narrowLength, 0);
        const int length =
            ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), narrowText.data(), narrowLength, nullptr, nullptr);
        if (length != narrowLength) {
            std::cerr<<"wide_to_utf8 convert string error.";
            return {};
        }

        return narrowText;
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    // See https://github.com/microsoft/OpenXR-MixedReality
    struct TextTextureInfo {
        TextTextureInfo(uint32_t width, uint32_t height)
            : Width(width)
            , Height(height) {
        }

        uint32_t Width;
        uint32_t Height;
        const wchar_t* FontName = L"Segoe UI";
        float FontSize = 18;
        float Margin = 0;
        std::array<float, 4> Foreground = { 1,1,1,1 };
        std::array<float, 4> Background = { 0,0,0,0 };
        DWRITE_TEXT_ALIGNMENT TextAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
        DWRITE_PARAGRAPH_ALIGNMENT ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
        DXGI_FORMAT TextFormat = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
        std::vector<std::tuple<std::wstring, uint32_t, uint32_t>> SpecialFontRanges = {};
        std::vector<std::wstring> CustomFontFilepaths = {};
    };

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    // See https://github.com/microsoft/OpenXR-MixedReality
    // Manages a texture which can be drawn to.
    //
    // Modified to support loading custom fonts
    class TextTexture {
    public:
        inline TextTexture(
            ID3D11Device4*          d3d11_device,
            ID2D1Factory2*          d2d_factory,
            ID2D1Device1*           d2d_device,
            ID2D1DeviceContext1*    d2d_context,
            IDWriteFactory5*        dwrite_factory,
            IDWriteFontCollection1* custom_font_collection,
            TextTextureInfo         textInfo,
            std::wstring const&     text)
            : m_textInfo(std::move(textInfo))
        {
            //
            // Create text format.
            //
            winrt::check_hresult(dwrite_factory->CreateTextFormat(m_textInfo.FontName,
                custom_font_collection,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                m_textInfo.FontSize,
                L"en-US",
                m_textFormat.put()));
            winrt::check_hresult(m_textFormat->SetTextAlignment(m_textInfo.TextAlignment));
            winrt::check_hresult(m_textFormat->SetParagraphAlignment(m_textInfo.ParagraphAlignment));

            //
            // Create text layout
            //
            auto wszText_ = text.c_str();
            auto cTextLength_ = (UINT32)wcslen(wszText_);
            winrt::check_hresult(dwrite_factory->CreateTextLayout(wszText_,
                cTextLength_,
                m_textFormat.get(),
                m_textInfo.Width,
                m_textInfo.Height,
                m_textLayout.put()));

            for (auto const& font_range : m_textInfo.SpecialFontRanges)
            {
                DWRITE_TEXT_RANGE range;
                range.startPosition = std::get<1>(font_range);
                range.length = std::get<2>(font_range);
                WCHAR const* font = std::get<0>(font_range).c_str();

                m_textLayout->SetFontFamilyName(font, range);
            }

            winrt::check_hresult(d2d_factory->CreateDrawingStateBlock(m_stateBlock.put()));

            //
            // Set up 2D rendering modes.
            //
            const D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(m_textInfo.TextFormat, D2D1_ALPHA_MODE_PREMULTIPLIED));

            const auto texDesc = CD3D11_TEXTURE2D_DESC(m_textInfo.TextFormat,
                m_textInfo.Width,
                m_textInfo.Height,
                1,
                1,
                D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                D3D11_USAGE_DEFAULT,
                0,
                1,
                0,
                0);
            winrt::check_hresult(d3d11_device->CreateTexture2D(&texDesc, nullptr, m_textDWriteTexture.put()));

            winrt::com_ptr<IDXGISurface> dxgiPerfBuffer = m_textDWriteTexture.as<IDXGISurface>();
            winrt::check_hresult(d2d_context->CreateBitmapFromDxgiSurface(dxgiPerfBuffer.get(), &bitmapProperties, m_d2dTargetBitmap.put()));

            d2d_context->SetTarget(m_d2dTargetBitmap.get());
            d2d_context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
            d2d_context->SetTransform(D2D1::Matrix3x2F::Identity());

            const auto& foreground = m_textInfo.Foreground;
            const D2D1::ColorF brushColor(std::get<0>(foreground), std::get<1>(foreground), std::get<2>(foreground), std::get<3>(foreground));
            winrt::check_hresult(d2d_context->CreateSolidColorBrush(brushColor, m_brush.put()));

            //
            // Draw to texture
            //
            d2d_context->SaveDrawingState(m_stateBlock.get());

            const D2D1_SIZE_F renderTargetSize = d2d_context->GetSize();
            d2d_context->BeginDraw();

            const auto& background = m_textInfo.Background;
            d2d_context->Clear(D2D1::ColorF(std::get<0>(background), std::get<1>(background), std::get<2>(background), std::get<3>(background)));

            if (!text.empty()) {
                d2d_context->DrawTextLayout(
                    { 0.0, 0.0 },
                    m_textLayout.get(),
                    m_brush.get()
                );
            }

            d2d_context->EndDraw();
            d2d_context->RestoreDrawingState(m_stateBlock.get());
        }

        ID3D11Texture2D* Texture() const
        {
            return m_textDWriteTexture.get();
        }

    private:
        const TextTextureInfo m_textInfo;
        winrt::com_ptr<ID2D1Bitmap1> m_d2dTargetBitmap;
        winrt::com_ptr<ID2D1SolidColorBrush> m_brush;
        winrt::com_ptr<ID2D1DrawingStateBlock> m_stateBlock;
        winrt::com_ptr<IDWriteTextFormat> m_textFormat;
        winrt::com_ptr<IDWriteTextLayout> m_textLayout;
        winrt::com_ptr<ID3D11Texture2D> m_textDWriteTexture;
    };

    void renderText(
        ID3D11Device4*          d3d11_device,
        ID2D1Factory2*          d2d_factory,
        ID2D1Device1*           d2d_device,
        ID2D1DeviceContext1*    d2d_context,
        IDWriteFactory5*        dwrite_factory,
        IDWriteFontCollection1* custom_font_collection,
        TextTextureInfo         textInfo,
        std::wstring const&     text,
        ID3D11Texture2D*        target_texture)
    {
        //
        // Create text format.
        //
        winrt::com_ptr<IDWriteTextFormat> textFormat;
        winrt::check_hresult(dwrite_factory->CreateTextFormat(textInfo.FontName,
            custom_font_collection,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            textInfo.FontSize,
            L"en-US",
            textFormat.put()));
        winrt::check_hresult(textFormat->SetTextAlignment(textInfo.TextAlignment));
        winrt::check_hresult(textFormat->SetParagraphAlignment(textInfo.ParagraphAlignment));

        //
        // Create text layout
        //
        winrt::com_ptr<IDWriteTextLayout> textLayout;
        auto wszText_ = text.c_str();
        auto cTextLength_ = (UINT32)wcslen(wszText_);
        winrt::check_hresult(dwrite_factory->CreateTextLayout(wszText_,
            cTextLength_,
            textFormat.get(),
            textInfo.Width,
            textInfo.Height,
            textLayout.put()));

        for (auto const& font_range : textInfo.SpecialFontRanges)
        {
            DWRITE_TEXT_RANGE range;
            range.startPosition = std::get<1>(font_range);
            range.length = std::get<2>(font_range);
            WCHAR const* font = std::get<0>(font_range).c_str();

            textLayout->SetFontFamilyName(font, range);
        }

        winrt::com_ptr<ID2D1DrawingStateBlock> stateBlock;
        winrt::check_hresult(d2d_factory->CreateDrawingStateBlock(stateBlock.put()));

        //
        // Set up 2D rendering modes.
        //
        const D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(textInfo.TextFormat, D2D1_ALPHA_MODE_PREMULTIPLIED));

        IDXGISurface* dxgiPerfBuffer;
        auto hr = target_texture->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiPerfBuffer);

        winrt::com_ptr<ID2D1Bitmap1> d2dTargetBitmap;
        winrt::check_hresult(d2d_context->CreateBitmapFromDxgiSurface(dxgiPerfBuffer, &bitmapProperties, d2dTargetBitmap.put()));

        d2d_context->SetTarget(d2dTargetBitmap.get());
        d2d_context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        d2d_context->SetTransform(D2D1::Matrix3x2F::Identity());

        winrt::com_ptr<ID2D1SolidColorBrush> brush;
        const auto& foreground = textInfo.Foreground;
        const D2D1::ColorF brushColor(std::get<0>(foreground), std::get<1>(foreground), std::get<2>(foreground), std::get<3>(foreground));
        winrt::check_hresult(d2d_context->CreateSolidColorBrush(brushColor, brush.put()));

        //
        // Draw to texture
        //
        d2d_context->SaveDrawingState(stateBlock.get());

        const D2D1_SIZE_F renderTargetSize = d2d_context->GetSize();
        d2d_context->BeginDraw();

        const auto& background = textInfo.Background;
        d2d_context->Clear(D2D1::ColorF(std::get<0>(background), std::get<1>(background), std::get<2>(background), std::get<3>(background)));

        if (!text.empty()) {
            d2d_context->DrawTextLayout(
                { 0.0, 0.0 },
                textLayout.get(),
                brush.get()
            );
        }

        d2d_context->EndDraw();
        d2d_context->RestoreDrawingState(stateBlock.get());
    }
}

EngineCore::Graphics::Dx11::ResourceManager::ResourceManager()
    : m_d3d11_device(nullptr), m_d3d11_device_context(nullptr), m_text_resources(nullptr)
{
}

void EngineCore::Graphics::Dx11::ResourceManager::init(ID3D11Device4* d3d11_device, ID3D11DeviceContext4* d3d11_device_context, std::vector<std::wstring> const& custom_font_filepaths)
{
    m_d3d11_device = d3d11_device;
    m_d3d11_device_context = d3d11_device_context;

    {
        m_text_resources = std::make_unique<TextResources>();

        D2D1_FACTORY_OPTIONS options{};

        winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, winrt::guid_of<ID2D1Factory2>(), &options, m_text_resources->m_d2d_factory.put_void()));
        winrt::check_hresult(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED, winrt::guid_of<IDWriteFactory5>(), reinterpret_cast<IUnknown**>(m_text_resources->m_dwrite_factory.put_void())));

        winrt::com_ptr <IDWriteFontCollection> system_font_collection;
        m_text_resources->m_dwrite_factory->GetSystemFontCollection(system_font_collection.put());

        // see https://learn.microsoft.com/en-us/windows/win32/directwrite/custom-font-sets-win10#creating-a-font-set-using-arbitrary-fonts-in-the-local-file-system
        winrt::com_ptr <IDWriteFontSetBuilder1> font_set_builder;
        winrt::check_hresult(m_text_resources->m_dwrite_factory->CreateFontSetBuilder(font_set_builder.put()));

        //std::vector<std::wstring> custom_font_filepaths = { utf8_to_wide(EngineCore::Utility::GetAppFolder().string() + "Resources/TheiaIcons.ttf") };

        for (auto const& font_filepath : custom_font_filepaths)
        {
            winrt::com_ptr <IDWriteFontFile> pFontFile;
            winrt::check_hresult(m_text_resources->m_dwrite_factory->CreateFontFileReference(font_filepath.c_str(), /* lastWriteTime*/ nullptr, pFontFile.put()));
            winrt::check_hresult(font_set_builder->AddFontFile(pFontFile.get()));
        }

        // get and add system font sets to font builder
        winrt::com_ptr <IDWriteFontSet> system_font_set;
        winrt::check_hresult(m_text_resources->m_dwrite_factory->GetSystemFontSet(system_font_set.put()));
        font_set_builder->AddFontSet(system_font_set.get());

        // build final font set
        winrt::com_ptr <IDWriteFontSet> font_set;
        winrt::check_hresult(font_set_builder->CreateFontSet(font_set.put()));

        //winrt::com_ptr <IDWriteFontCollection1> custom_font_collection;
        winrt::check_hresult(m_text_resources->m_dwrite_factory->CreateFontCollectionFromFontSet(font_set.get(), m_text_resources->m_custom_font_collection.put()));

        IDXGIDevice* pDXGIDevice;
        auto hr = d3d11_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
        winrt::check_hresult(m_text_resources->m_d2d_factory->CreateDevice(pDXGIDevice, m_text_resources->m_d2d_device.put()));
        winrt::check_hresult(m_text_resources->m_d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_text_resources->m_d2d_context.put()));
    }
}

void EngineCore::Graphics::Dx11::ResourceManager::clearAllResources()
{
    m_buffers.clear();
    m_meshes.clear();
    m_shader_programs.clear();
    m_textures_2d.clear();
    m_textures_3d.clear();
}

ResourceID EngineCore::Graphics::Dx11::ResourceManager::allocateMeshAsync(
    std::string const & name,
    size_t vertex_cnt,
    size_t index_cnt,
    //std::shared_ptr<GenericVertexLayout> const & vertex_layout,
    std::shared_ptr<std::vector<VertexLayout>> const& vertex_layout,
    DXGI_FORMAT const index_type,
    D3D_PRIMITIVE_TOPOLOGY const mesh_type)
{
    size_t idx = m_meshes.size();
    ResourceID rsrc_id = generateResourceID();

    {
        std::unique_lock<std::shared_mutex> lock(m_meshes_mutex);
        m_meshes.push_back(Resource<dxowl::Mesh>(rsrc_id));
        m_id_to_mesh_idx.insert(std::pair<unsigned int, size_t>(m_meshes.back().id.value(), idx));
    }

    std::async([this, idx, vertex_cnt, index_cnt, vertex_layout, index_type, mesh_type](){

        std::shared_lock<std::shared_mutex> lock(m_meshes_mutex);

        // TODO create DirectX vertex descriptor
        //Graphics::Dx11::VertexDescriptor vertex_descriptor(*vertex_layout);
        std::vector<dxowl::VertexDescriptor> vertex_descriptor = (*vertex_layout);

        // TODO get number of buffer required for vertex layout and compute byte sizes
        std::vector<void*> vertex_data_ptrs(vertex_descriptor.size(), nullptr);
        std::vector<size_t> vertex_data_buffer_byte_sizes;
        vertex_data_buffer_byte_sizes.reserve(vertex_descriptor.size());

        for (auto const& vl : vertex_descriptor){
            vertex_data_buffer_byte_sizes.push_back(computeVertexByteSize(vl) * vertex_cnt);
        }

        size_t index_data_byte_size = 4 * index_cnt; //TODO support different index formats


        this->m_meshes[idx].resource = std::make_unique<dxowl::Mesh>(
            m_d3d11_device,
            vertex_data_ptrs,
            vertex_data_buffer_byte_sizes,
            nullptr,
            index_data_byte_size,
            vertex_descriptor,
            index_type,
            mesh_type);

        this->m_meshes[idx].state = READY;
    });

    return m_meshes.back().id;
}

// Workaround for co_await code generation with optimizations enabled
//#pragma optimize( "", off )
//std::future<ResourceID> ResourceManager::createShaderProgramAsync(
ResourceID ResourceManager::createShaderProgramAsync(
    std::string const & name,
    std::shared_ptr<std::vector<ResourceManager::ShaderFilename>> shader_filenames,
    std::shared_ptr<std::vector<dxowl::VertexDescriptor>> vertex_layout)
{
    {
        std::shared_lock<std::shared_mutex> shader_lock(m_shader_programs_mutex);
        //TODO search if shader setup already exists
        for (size_t shader_idx = 0; shader_idx < m_shader_programs_identifier.size(); ++shader_idx)
        {
            if (m_shader_programs_identifier[shader_idx] == name)
            {
                //TODO check shader input layout
                //co_return m_shader_programs[shader_idx].id;
                return m_shader_programs[shader_idx].id;
            }
        }
    }

    size_t idx = m_shader_programs.size();
    ResourceID rsrc_id = generateResourceID();

    {
        std::unique_lock<std::shared_mutex> shader_lock(m_shader_programs_mutex);
        m_shader_programs.push_back(Resource<dxowl::ShaderProgram>(rsrc_id));
        m_shader_programs_identifier.push_back(name);
        m_id_to_shader_program_idx.insert(std::pair<unsigned int, size_t>(m_shader_programs.back().id.value(), idx));
    }

    auto rsrc_mngr_ptr = this;

    //co_await std::async([&rsrc_mngr_ref, idx, shader_filenames, vertex_layout]() ->std::future <void> {
    std::async([rsrc_mngr_ptr, idx, shader_filenames, vertex_layout]() {
        std::vector<byte> vertex_shader;
        std::vector<byte> geometry_shader;
        std::vector<byte> pixel_shader;

        for (auto& shader_filename : *shader_filenames)
        {
            switch (shader_filename.second)
            {
            case dxowl::ShaderProgram::VertexShader:
                //vertex_shader = co_await ReadDataAsync(shader_filename.first);
                vertex_shader = Utility::ReadFileBytes(std::filesystem::path(shader_filename.first));
                break;
            case dxowl::ShaderProgram::PixelShader:
                //pixel_shader = co_await ReadDataAsync(shader_filename.first);
                pixel_shader = Utility::ReadFileBytes(std::filesystem::path(shader_filename.first));
                break;
            case dxowl::ShaderProgram::GeometryShader:
                //geometry_shader = co_await ReadDataAsync(shader_filename.first);
                geometry_shader = Utility::ReadFileBytes(std::filesystem::path(shader_filename.first));
                break;
            default:
                break;
            }
        }
        std::unique_lock<std::shared_mutex> shader_lock(rsrc_mngr_ptr->m_shader_programs_mutex);

        rsrc_mngr_ptr->m_shader_programs[idx].resource = std::make_unique<dxowl::ShaderProgram>(
            rsrc_mngr_ptr->getD3D11Device(),
            *vertex_layout,
            vertex_shader,
            geometry_shader,
            pixel_shader);

        rsrc_mngr_ptr->m_shader_programs[idx].state = READY;
    });

    //co_return m_shader_programs.back().id;
    return m_shader_programs.back().id;
}

ResourceID EngineCore::Graphics::Dx11::ResourceManager::createShaderProgram(
    std::string const & name,
    std::vector<ShaderData> const & shader_bytedata,
    std::vector<dxowl::VertexDescriptor> const & vertex_layout)
{
    std::unique_lock<std::shared_mutex> shader_lock(m_shader_programs_mutex);

    //TODO search if shader setup already exists
    for (size_t shader_idx = 0; shader_idx < m_shader_programs_identifier.size(); ++shader_idx)
    {
        if (m_shader_programs_identifier[shader_idx] == name)
        {
            //TODO check shader input layout
            return m_shader_programs[shader_idx].id;
        }
    }

    size_t idx = m_shader_programs.size();
    ResourceID rsrc_id = generateResourceID();
    m_shader_programs.push_back(Resource<dxowl::ShaderProgram>(rsrc_id));
    m_shader_programs_identifier.push_back(name);

    m_id_to_shader_program_idx.insert(std::pair<unsigned int, size_t>(m_shader_programs.back().id.value(), idx));

    std::pair<const void*, size_t> vertex_shader = {nullptr,0};
    std::pair<const void*,size_t> geometry_shader = { nullptr,0 };
    std::pair<const void*,size_t> pixel_shader = { nullptr,0 };

    for (auto& shader_filename : shader_bytedata)
    {
        switch (std::get<2>(shader_filename))
        {
        case dxowl::ShaderProgram::VertexShader:
            vertex_shader.first = std::get<0>(shader_filename);
            vertex_shader.second = std::get<1>(shader_filename);
            break;
        case dxowl::ShaderProgram::PixelShader:
            pixel_shader.first = std::get<0>(shader_filename);
            pixel_shader.second = std::get<1>(shader_filename);
            break;
        case dxowl::ShaderProgram::GeometryShader:
            geometry_shader.first = std::get<0>(shader_filename);
            geometry_shader.second = std::get<1>(shader_filename);
            break;
        default:
            break;
        }
    }

    m_shader_programs[idx].resource = std::make_unique<dxowl::ShaderProgram>(
        m_d3d11_device,
        vertex_layout,
        vertex_shader.first,
        vertex_shader.second,
        geometry_shader.first,
        geometry_shader.second,
        pixel_shader.first,
        pixel_shader.second);

    m_shader_programs[idx].state = READY;

    return m_shader_programs.back().id;
}


ResourceID EngineCore::Graphics::Dx11::ResourceManager::createTextTexture2DAsync(
    std::string const& name,
    std::wstring const& text,
    FontInfo const& font_info,
    D3D11_TEXTURE2D_DESC const& desc,
    bool generate_mipmap)
{
    std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);

    size_t idx = m_textures_2d.size();
    ResourceID rsrc_id = generateResourceID();
    m_textures_2d.push_back(Resource<dxowl::Texture2D>(rsrc_id));

    addTextureIndex(rsrc_id.value(), name, idx);

    m_renderThread_tasks.push(
        [this, idx, text, desc, generate_mipmap, font_info]() {

            std::vector<const void*> data_ptrs;

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

            TextTextureInfo text_info(desc.Width, desc.Height);
            text_info.FontName = font_info.default_font_name.c_str();
            text_info.FontSize = font_info.font_size;
            text_info.Foreground = font_info.foreground;
            text_info.Background = font_info.background;
            text_info.TextAlignment = font_info.text_alignment;
            text_info.SpecialFontRanges = font_info.special_font_ranges;
            text_info.CustomFontFilepaths = font_info.custom_font_filepaths;
            //  std::unique_ptr<TextTexture> text_to_texture
            //      = std::make_unique<TextTexture>(
            //          m_d3d11_device,
            //          m_text_resources->m_d2d_factory.get(),
            //          m_text_resources->m_d2d_device.get(),
            //          m_text_resources->m_d2d_context.get(),
            //          m_text_resources->m_dwrite_factory.get(),
            //          m_text_resources->m_custom_font_collection.get(),
            //          text_info,
            //          text);
            //
            //  // copy rendered text texture
            //  {
            //      D3D11_BOX src_region;
            //      src_region.left = 0;
            //      src_region.right = desc.Width;
            //      src_region.top = 0;
            //      src_region.bottom = desc.Height;
            //      src_region.front = 0;
            //      src_region.back = 1;
            //
            //      ID3D11Resource* src_rsrc = text_to_texture->Texture();
            //      ID3D11Resource* dest_rsrc;
            //      this->m_textures_2d[idx].resource->getShaderResourceView()->GetResource(&dest_rsrc);
            //      m_d3d11_device_context->CopySubresourceRegion(dest_rsrc, 0, 0, 0, 0, src_rsrc, 0, &src_region);
            //  }
            renderText(
                m_d3d11_device,
                m_text_resources->m_d2d_factory.get(),
                m_text_resources->m_d2d_device.get(),
                m_text_resources->m_d2d_context.get(),
                m_text_resources->m_dwrite_factory.get(),
                m_text_resources->m_custom_font_collection.get(),
                text_info,
                text,
                this->m_textures_2d[idx].resource->getTexture().Get());

            this->m_textures_2d[idx].state = READY;
        }
    );

    return m_textures_2d[idx].id;
}

void EngineCore::Graphics::Dx11::ResourceManager::updateTextTexture2D(
    ResourceID rsrc_id,
    std::wstring const& text,
    FontInfo const& font_info)
{
    WeakResource<dxowl::Texture2D> texture = getTexture2DResource(rsrc_id);

    if (texture.state == READY) {
        auto desc = texture.resource->getTextureDesc();

        TextTextureInfo text_info(desc.Width, desc.Height);
        text_info.FontName = font_info.default_font_name.c_str();
        text_info.FontSize = font_info.font_size;
        text_info.Foreground = font_info.foreground;
        text_info.Background = font_info.background;
        text_info.TextAlignment = font_info.text_alignment;
        text_info.SpecialFontRanges = font_info.special_font_ranges;
        text_info.CustomFontFilepaths = font_info.custom_font_filepaths;
        //std::unique_ptr<TextTexture> text_to_texture
        //    = std::make_unique<TextTexture>(
        //        m_d3d11_device,
        //        m_text_resources->m_d2d_factory.get(),
        //        m_text_resources->m_d2d_device.get(),
        //        m_text_resources->m_d2d_context.get(),
        //        m_text_resources->m_dwrite_factory.get(),
        //        m_text_resources->m_custom_font_collection.get(),
        //        text_info,
        //        text);
        //
        //// copy rendered text texture
        //{
        //    D3D11_BOX src_region;
        //    src_region.left = 0;
        //    src_region.right = desc.Width;
        //    src_region.top = 0;
        //    src_region.bottom = desc.Height;
        //    src_region.front = 0;
        //    src_region.back = 1;
        //
        //    ID3D11Resource* src_rsrc = text_to_texture->Texture();
        //    ID3D11Resource* dest_rsrc;
        //    texture.resource->getShaderResourceView()->GetResource(&dest_rsrc);
        //    m_d3d11_device_context->CopySubresourceRegion(dest_rsrc, 0, 0, 0, 0, src_rsrc, 0, &src_region);
        //}
        renderText(
            m_d3d11_device,
            m_text_resources->m_d2d_factory.get(),
            m_text_resources->m_d2d_device.get(),
            m_text_resources->m_d2d_context.get(),
            m_text_resources->m_dwrite_factory.get(),
            m_text_resources->m_custom_font_collection.get(),
            text_info,
            text,
            texture.resource->getTexture().Get());
    }
}

void EngineCore::Graphics::Dx11::ResourceManager::updateTextTexture2DAsync(ResourceID rsrc_id, std::wstring const& text, FontInfo const& font_info)
{

    m_renderThread_tasks.push(
        [this, rsrc_id, text, font_info]() {
            updateTextTexture2D(rsrc_id, text, font_info);
        }
    );
}

Vec2 EngineCore::Graphics::Dx11::ResourceManager::estimateTextTextureSize(FontInfo const& font_info, float dpi, size_t line_length, size_t lines)
{
    Vec2 size = { 0.0f, 0.0f };
    UINT32 index = 0;
    BOOL exists = false;

    winrt::com_ptr <IDWriteFontFamily> pFontFamily;
    winrt::check_hresult(m_text_resources->m_custom_font_collection->FindFamilyName(font_info.default_font_name.c_str(), &index, &exists));
    if (exists)
    {
        winrt::check_hresult(m_text_resources->m_custom_font_collection->GetFontFamily(index, pFontFamily.put()));

        winrt::com_ptr <IDWriteFont> pFont;
        winrt::check_hresult(pFontFamily->GetFont(0, pFont.put()));

        DWRITE_FONT_METRICS font_metrics;
        pFont->GetMetrics(&font_metrics);
        font_metrics.designUnitsPerEm;

        winrt::com_ptr <IDWriteFontFace> pFontFace;
        winrt::check_hresult(pFont->CreateFontFace(pFontFace.put()));

        UINT16 glyph_count = pFontFace->GetGlyphCount();

        std::vector<UINT16> glyph_indices(glyph_count);
        std::vector<DWRITE_GLYPH_METRICS> glyph_metrics(glyph_count);

        for (UINT16 i = 0; i < glyph_count; ++i) {
            glyph_indices[i] = i;
        }

        winrt::check_hresult(pFontFace->GetDesignGlyphMetrics(glyph_indices.data(), glyph_count, glyph_metrics.data(), FALSE));

        UINT32 max_width = 0;
        UINT32 max_height = 0;

        for (const auto& metrics : glyph_metrics) {
            max_width = std::max(max_width, metrics.advanceWidth);
            max_height = std::max(max_height, metrics.advanceHeight);
        }

        float font_size = font_info.font_size * (dpi / 72.0f);

        size.x = std::ceil(max_width * font_size / font_metrics.designUnitsPerEm) * line_length;
        size.y = std::ceil(max_height * font_size / font_metrics.designUnitsPerEm) * lines;
    }
    return size;
}