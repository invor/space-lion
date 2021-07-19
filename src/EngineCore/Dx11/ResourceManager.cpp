#include "ResourceManager.hpp"

#include <coroutine>

#include <winrt/Windows.Storage.h>

#include <filesystem>
#include <fstream>

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

	std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
		bool fileExists = false;
		try {
			std::ifstream file;
			file.exceptions(std::ios::failbit | std::ios::badbit);
			file.open(path, std::ios::binary | std::ios::ate);
			fileExists = true;
			// If tellg fails then it will throw an exception instead of returning -1.
			std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(data.data()), data.size());
			return data;
		}
		catch (const std::ios::failure&) {
			// The exception only knows that the failbit was set so it doesn't contain anything useful.
			//throw std::runtime_error(fmt::format("Failed to {} file: {}", fileExists ? "read" : "open", path.string()));
		}
	}
}

EngineCore::Graphics::Dx11::ResourceManager::ResourceManager(ID3D11Device4* d3d11_device, ID3D11DeviceContext4* d3d11_device_context)
	: m_d3d11_device(d3d11_device), m_d3d11_device_context(d3d11_device_context)
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
    std::shared_ptr<VertexLayout> const& vertex_layout,
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
        dxowl::VertexDescriptor vertex_descriptor = (*vertex_layout);

		// TODO get number of buffer required for vertex layout and compute byte sizes
		std::vector<void*> vertex_data_ptrs(vertex_descriptor.attributes.size(), nullptr);
		std::vector<size_t> vertex_data_buffer_byte_sizes(vertex_descriptor.attributes.size());

		for (size_t attrib_idx = 0; attrib_idx < vertex_descriptor.attributes.size(); ++attrib_idx){
			vertex_data_buffer_byte_sizes[attrib_idx] = dxowl::computeAttributeByteSize(vertex_descriptor.attributes[attrib_idx]) * vertex_cnt;
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
	std::shared_ptr<dxowl::VertexDescriptor> const& vertex_layout)
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
				vertex_shader = ReadFileBytes(std::filesystem::path(shader_filename.first));
				break;
			case dxowl::ShaderProgram::PixelShader:
				//pixel_shader = co_await ReadDataAsync(shader_filename.first);
				pixel_shader = ReadFileBytes(std::filesystem::path(shader_filename.first));
				break;
			case dxowl::ShaderProgram::GeometryShader:
				//geometry_shader = co_await ReadDataAsync(shader_filename.first);
				geometry_shader = ReadFileBytes(std::filesystem::path(shader_filename.first));
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
	dxowl::VertexDescriptor const & vertex_layout)
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
