#include "ResourceManager.hpp"

#include <winrt/Windows.Storage.h>

#include <iostream>
#include <codecvt>
#include <filesystem>
#include <fstream>

#include <dxgi.h>
#include <d2d1_2.h>
#include <dwrite_2.h>

#include "ResourceLoading.hpp"

using namespace EngineCore::Graphics;
using namespace EngineCore::Graphics::Dx11;

namespace {
	// Function that reads from a binary file asynchronously.
	//inline std::future<std::vector<byte>> ReadDataAsync(const std::wstring_view& filename)
	//{
	//	using namespace winrt::Windows::Storage;
	//	using namespace winrt::Windows::Storage::Streams;
	//
	//	IBuffer fileBuffer = co_await PathIO::ReadBufferAsync(filename);
	//
	//	std::vector<byte> returnBuffer;
	//	returnBuffer.resize(fileBuffer.Length());
	//	DataReader::FromBuffer(fileBuffer).ReadBytes(winrt::array_view<uint8_t>(returnBuffer));
	//	co_return returnBuffer;
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
	};

	// Copyright (c) Microsoft Corporation.
	// Licensed under the MIT License.
	// See https://github.com/microsoft/OpenXR-MixedReality
	// Manages a texture which can be drawn to.
	class TextTexture {
	public:
		inline TextTexture(
			ID3D11Device4* d3d11_device,
			ID3D11DeviceContext4* d3d11_device_context,
			TextTextureInfo textInfo)
			: m_textInfo(std::move(textInfo))
		{
			D2D1_FACTORY_OPTIONS options{};
			
			winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, winrt::guid_of<ID2D1Factory2>(), &options, m_d2dFactory.put_void()));
			winrt::check_hresult(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED, winrt::guid_of<IDWriteFactory2>(), reinterpret_cast<IUnknown**>(m_dwriteFactory.put_void())));

			IDXGIDevice* pDXGIDevice;
			auto hr = d3d11_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
			winrt::check_hresult(m_d2dFactory->CreateDevice(pDXGIDevice, m_d2dDevice.put()));
			winrt::check_hresult(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_d2dContext.put()));

			//
			// Create text format.
			//
			winrt::check_hresult(m_dwriteFactory->CreateTextFormat(m_textInfo.FontName,
				nullptr,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				m_textInfo.FontSize,
				L"en-US",
				m_textFormat.put()));
			winrt::check_hresult(m_textFormat->SetTextAlignment(m_textInfo.TextAlignment));
			winrt::check_hresult(m_textFormat->SetParagraphAlignment(m_textInfo.ParagraphAlignment));

			winrt::check_hresult(m_d2dFactory->CreateDrawingStateBlock(m_stateBlock.put()));

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
			winrt::check_hresult(m_d2dContext->CreateBitmapFromDxgiSurface(dxgiPerfBuffer.get(), &bitmapProperties, m_d2dTargetBitmap.put()));

			m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
			m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
			m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

			const auto& foreground = m_textInfo.Foreground;
			const D2D1::ColorF brushColor(std::get<0>(foreground), std::get<1>(foreground), std::get<2>(foreground), std::get<3>(foreground));
			winrt::check_hresult(m_d2dContext->CreateSolidColorBrush(brushColor, m_brush.put()));
		}

		inline void Draw(std::string text)
		{
			m_d2dContext->SaveDrawingState(m_stateBlock.get());

			const D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
			m_d2dContext->BeginDraw();

			const auto& background = m_textInfo.Background;
			m_d2dContext->Clear(D2D1::ColorF(std::get<0>(background), std::get<1>(background), std::get<2>(background), std::get<3>(background)));

			if (!text.empty()) {
				const auto& margin = m_textInfo.Margin;
				std::wstring wtext = utf8_to_wide(text);
				m_d2dContext->DrawText(wtext.c_str(),
					(UINT32)wtext.size(),
					m_textFormat.get(),
					D2D1::RectF(m_textInfo.Margin,
						m_textInfo.Margin,
						renderTargetSize.width - m_textInfo.Margin * 2,
						renderTargetSize.height - m_textInfo.Margin * 2),
					m_brush.get());
			}

			m_d2dContext->EndDraw();

			m_d2dContext->RestoreDrawingState(m_stateBlock.get());
		}

		ID3D11Texture2D* Texture() const
		{
			return m_textDWriteTexture.get();
		}

	private:
		const TextTextureInfo m_textInfo;
		winrt::com_ptr<ID2D1Factory2> m_d2dFactory;
		winrt::com_ptr<ID2D1Device1> m_d2dDevice;
		winrt::com_ptr<ID2D1DeviceContext1> m_d2dContext;
		winrt::com_ptr<ID2D1Bitmap1> m_d2dTargetBitmap;
		winrt::com_ptr<ID2D1SolidColorBrush> m_brush;
		winrt::com_ptr<ID2D1DrawingStateBlock> m_stateBlock;
		winrt::com_ptr<IDWriteFactory2> m_dwriteFactory;
		winrt::com_ptr<IDWriteTextFormat> m_textFormat;
		winrt::com_ptr<ID3D11Texture2D> m_textDWriteTexture;
	};

}

EngineCore::Graphics::Dx11::ResourceManager::ResourceManager()
	: m_d3d11_device(nullptr), m_d3d11_device_context(nullptr)
{
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
	std::shared_ptr<std::vector<ResourceManager::ShaderFilename>> const& shader_filenames,
	std::shared_ptr<std::vector<dxowl::VertexDescriptor>> const& vertex_layout)
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

	auto& rsrc_mngr_ref = *this;

	//co_await std::async([&rsrc_mngr_ref, idx, shader_filenames, vertex_layout]() ->std::future <void> {
	std::async([&rsrc_mngr_ref, idx, shader_filenames, vertex_layout]() {

		auto& cached_rsrc_mngr_ref = rsrc_mngr_ref;
		auto cached_idx = idx;
		auto cached_shader_filenames = shader_filenames;
		auto cached_vertex_layout = vertex_layout;

		std::vector<byte> vertex_shader;
		std::vector<byte> geometry_shader;
		std::vector<byte> pixel_shader;

		for (auto& shader_filename : *cached_shader_filenames)
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
		std::unique_lock<std::shared_mutex> shader_lock(cached_rsrc_mngr_ref.m_shader_programs_mutex);

		cached_rsrc_mngr_ref.m_shader_programs[cached_idx].resource = std::make_unique<dxowl::ShaderProgram>(
			cached_rsrc_mngr_ref.getD3D11Device(),
			*cached_vertex_layout,
			vertex_shader,
			geometry_shader,
			pixel_shader);

		cached_rsrc_mngr_ref.m_shader_programs[cached_idx].state = READY;
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
	std::string const& text,
	D3D11_TEXTURE2D_DESC const& desc,
	bool generate_mipmap)
{
	std::unique_lock<std::shared_mutex> lock(m_textures_2d_mutex);

	size_t idx = m_textures_2d.size();
	ResourceID rsrc_id = generateResourceID();
	m_textures_2d.push_back(Resource<dxowl::Texture2D>(rsrc_id));

	addTextureIndex(rsrc_id.value(), name, idx);

	m_renderThread_tasks.push(
		[this, idx, text, desc, generate_mipmap]() {

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

			std::unique_ptr<TextTexture> text_to_texture
				= std::make_unique<TextTexture>(m_d3d11_device, m_d3d11_device_context, TextTextureInfo(desc.Width,desc.Height));
			text_to_texture->Draw(text);

			// copy rendered text texture
			{
				D3D11_BOX src_region;
				src_region.left = 0;
				src_region.right = 512;
				src_region.top = 0;
				src_region.bottom = 512;
				src_region.front = 0;
				src_region.back = 1;

				ID3D11Resource* src_rsrc = text_to_texture->Texture();
				ID3D11Resource* dest_rsrc;
				this->m_textures_2d[idx].resource->getShaderResourceView()->GetResource(&dest_rsrc);
				m_d3d11_device_context->CopySubresourceRegion(dest_rsrc, 0, 0, 0, 0, src_rsrc, 0, &src_region);
			}

			this->m_textures_2d[idx].state = READY;
		}
	);

	return m_textures_2d[idx].id;
}