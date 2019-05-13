#include "WorldState.hpp"

#include "gltfSceneLoading.hpp"

EngineCore::WorldState::WorldState(ResourceManager* resource_manager)
	: m_material_manager(resource_manager), m_mesh_manager(resource_manager), m_airplane_physics_manager(128, *this)
{}

void EngineCore::WorldState::createDebugScene()
{
	/*Entity debug_entity = m_entity_manager.create();

	m_transform_manager.addComponent(debug_entity,Vec3(0.0f,0.0f,-2.0f),Quat::CreateFromAxisAngle(Vec3(0.0f,1.0f,0.0f),0.0f), Vec3(1.0f, 1.0f, 1.0f));

	auto triangle_vertices = std::make_shared<std::vector<std::vector<float>>>();
	(*triangle_vertices) = { {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
								{-0.25f, 0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f},
								{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f} };

	auto triangle_indices = std::make_shared<std::vector<uint32_t>>(3);
	(*triangle_indices) = { 0,1,2 };

	auto vertex_layout = std::make_shared<VertexLayout>();
	vertex_layout->strides = { 12, 12, 8 };
	vertex_layout->attributes = {
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	auto mesh_rsrc = m_mesh_manager.addComponent(debug_entity, "debug_mesh", triangle_vertices, triangle_indices, vertex_layout, DXGI_FORMAT_R32_UINT, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	typedef std::pair < std::wstring, Graphics::Dx11::ShaderProgram::ShaderType > ShaderFilename;

	auto shader_names = std::make_shared<std::vector<ShaderFilename>>(
		std::initializer_list<ShaderFilename>{
			{ L"ms-appx:///gltfVertexShader.cso", Graphics::Dx11::ShaderProgram::VertexShader },
			{ L"ms-appx:///GeometryShader.cso", Graphics::Dx11::ShaderProgram::GeometryShader },
			{ L"ms-appx:///PixelShader.cso", Graphics::Dx11::ShaderProgram::PixelShader }
		}
	);

	auto shader_rsrc = m_resource_manager.createShaderProgramAsync(
		"triangle_debug_shader",
		shader_names,
		vertex_layout);

	m_material_manager.addComponent(debug_entity, "debug_material", shader_rsrc);

	m_render_task_manager.addComponent(debug_entity, mesh_rsrc, 0, shader_rsrc, 0);*/
}

//std::future<void>  EngineCore::WorldState::createDebugSceneGLTF(std::function<void()> callback)
//{
	//{
	//	D3D11_TEXTURE2D_DESC texDesc;
	//	texDesc.Width = 3;
	//	texDesc.Height = 3;
	//	texDesc.MipLevels = 1;
	//	texDesc.ArraySize = 6;
	//	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	texDesc.CPUAccessFlags = 0;
	//	texDesc.SampleDesc.Count = 1;
	//	texDesc.SampleDesc.Quality = 0;
	//	texDesc.Usage = D3D11_USAGE_DEFAULT;
	//	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//	texDesc.CPUAccessFlags = 0;
	//	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	//	SMViewDesc.Format = texDesc.Format;
	//	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	//	SMViewDesc.TextureCube.MostDetailedMip = 0;

	//	auto data = std::make_shared<std::vector<std::vector<std::array<std::byte, 4>>>>(6); // 6 images of with 4 bytes per texel

	//	std::array<std::byte, 4> top = { std::byte(10), std::byte(55), std::byte(150), std::byte(255) };
	//	std::array<std::byte, 4> high = { std::byte(26), std::byte(80), std::byte(165), std::byte(255) };
	//	std::array<std::byte, 4> mid = { std::byte(50), std::byte(115), std::byte(227), std::byte(255) };
	//	std::array<std::byte, 4> low = { std::byte(90), std::byte(145), std::byte(226), std::byte(255) };
	//	std::array<std::byte, 4> ground = { std::byte(110), std::byte(105), std::byte(93), std::byte(255) };

	//	(*data)[0] = { high,high,high,mid,mid,mid,low,low,low };
	//	(*data)[1] = { high,high,high,mid,mid,mid,low,low,low };
	//	(*data)[2] = { top,top,top,top,top,top,top,top,top };
	//	(*data)[3] = { ground,ground,ground,ground,ground,ground,ground,ground,ground };
	//	(*data)[4] = { high,high,high,mid,mid,mid,low,low,low };
	//	(*data)[5] = { high,high,high,mid,mid,mid,low,low,low };

	//	//for (int cubeMapFaceIndex = 0; cubeMapFaceIndex < 6; cubeMapFaceIndex++)
	//	//{
	//	//	(*data)[cubeMapFaceIndex].resize(512 * 512);
	//	//
	//	//	std::array<std::byte, 4> red_texel = { std::byte(255), std::byte(0), std::byte(0), std::byte(255) };
	//	//
	//	//	// fill with red color  
	//	//	std::fill(
	//	//		(*data)[cubeMapFaceIndex].begin(),
	//	//		(*data)[cubeMapFaceIndex].end(),
	//	//		red_texel);
	//	//}

	//	m_resource_manager.createTexture2DAsync("debug_cubemap", data, texDesc, SMViewDesc);
	//}

	//co_await EngineCore::Utility::ResourceLoading::loadScene(L"ms-appx:///Assets/gltf_export_scene_20190312.xml", *this);
	//callback();
	////EngineCore::Utility::ResourceLoading::loadScene(L"ms-appx:///Assets/debug_scene.xml", *this);
//}
