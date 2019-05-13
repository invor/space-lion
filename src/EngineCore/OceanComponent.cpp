#include "OceanComponent.hpp"

// glowl includes
#include "glowl.h"

#include "GlobalEngineCore.hpp"
#include "DeferredRenderingPipeline.hpp"
#include "EntityManager.hpp"
#include "Frame.hpp"
#include "ResourceManager.hpp"

#include "GlobalRenderingComponents.hpp"
#include "AtmosphereComponent.hpp"

#include "GlobalCoreComponents.hpp"
#include "SunlightComponent.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"

#include "GlobalTools.hpp"
#include "EditorUI.hpp"

#include <functional>


OceanComponentManager::OceanComponentManager()
	: ocean_fresnel_lut(nullptr)
{
	time = std::chrono::high_resolution_clock::now();

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask(std::bind(&OceanComponentManager::createGpuResources, this));

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask(std::bind(&OceanComponentManager::precomputeFresnel, this));
}

OceanComponentManager::~OceanComponentManager()
{
}

void OceanComponentManager::addComponent(Entity entity, float wave_height, float patch_size, uint grid_size)
{
	uint idx = static_cast<int>(m_data.size());

	m_data.push_back(Data(entity, wave_height, patch_size, grid_size));

	m_data[idx].gaussian_noise = nullptr;
	m_data[idx].tilde_h0_of_k = nullptr;
	m_data[idx].tilde_h0_of_minus_k = nullptr;
	m_data[idx].twiddle = nullptr;
	m_data[idx].spectrum_x_displacement = nullptr;
	m_data[idx].spectrum_y_displacement = nullptr;
	m_data[idx].spectrum_z_displacement = nullptr;

	m_data[idx].displacement = nullptr;
	m_data[idx].normal = nullptr;

	GEngineCore::renderingPipeline().addSingleExecutionGpuTask(std::bind(&OceanComponentManager::createComponentGpuResources,this,idx));
	GEngineCore::renderingPipeline().addSingleExecutionGpuTask(std::bind(&OceanComponentManager::computeIntialSpectrum, this, idx));
	GEngineCore::renderingPipeline().addSingleExecutionGpuTask(std::bind(&OceanComponentManager::computeTwiddleTexture, this, idx));
}

std::pair<bool, uint> OceanComponentManager::getIndex(uint entity_id)
{
	auto search = m_index_map.find(entity_id);

	std::pair<bool, uint> rtn;

	rtn.first = (search != m_index_map.end()) ? true : false;
	rtn.second = search->second;

	return rtn;
}

void OceanComponentManager::updateOceanHeightfields()
{
	for (uint i=0; i< m_data.size(); i++)
	{

		auto t = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> d = t - time;

		//std::cout << "Time: " << d.count()/1000.0 << " s" << std::endl;

		//TODO get time!
		updateSpectrum(i,d.count()/1000.0);
		computeIFFT(i);
		updateDisplacementNormal(i);
	}
}

void OceanComponentManager::drawOceanSurfaces(Frame const& frame)
{
	ocean_surface_prgm->use();

	/* Get information on active camera */
	Mat4x4 view_matrix = frame.m_view_matrix;
	Mat4x4 proj_matrix = glm::perspective(frame.m_fovy, frame.m_aspect_ratio, 0.01f, 10000.0f);

	/* Bind shader program and set per program uniforms */
	ocean_surface_prgm->setUniform("projection_matrix", proj_matrix);
	ocean_surface_prgm->setUniform("view_matrix", view_matrix);

	glActiveTexture(GL_TEXTURE0);
	if( m_data[0].displacement != nullptr)
		m_data[0].displacement->bindTexture();
	ocean_surface_prgm->setUniform("displacement_tx2D", 0);

	glActiveTexture(GL_TEXTURE1);
	if (m_data[0].normal != nullptr)
		m_data[0].normal->bindTexture();
	ocean_surface_prgm->setUniform("normal_tx2D", 1);

	glActiveTexture(GL_TEXTURE2);
	if (ocean_fresnel_lut != nullptr)
		ocean_fresnel_lut->bindTexture();
	ocean_surface_prgm->setUniform("fresnel_lut_tx2D", 2);

	glActiveTexture(GL_TEXTURE3);
	GEngineCore::renderingPipeline().getGBuffer()->bindColorbuffer(1); // depth value
	ocean_surface_prgm->setUniform("depth_tx2D", 3);

	ocean_surface_prgm->setUniform("gBuffer_resolution", Vec2(GEngineCore::renderingPipeline().getGBuffer()->getWidth(), GEngineCore::renderingPipeline().getGBuffer()->getHeight()) );

	// TODO Add sky lighting
	const Material* atmosphere_mtl = GRenderingComponents::atmosphereManager().getMaterial(0); //DON'T SIMPLY ASSUME 0

	glActiveTexture(GL_TEXTURE4);
	atmosphere_mtl->getTextures()[1]->bindTexture();
	ocean_surface_prgm->setUniform("atmosphere_rayleigh_scattering", 5);

	glActiveTexture(GL_TEXTURE5);
	atmosphere_mtl->getTextures()[2]->bindTexture();
	ocean_surface_prgm->setUniform("atmosphere_mie_scattering", 4);

	int sun_count = frame.m_sunlight_positions.size();
	for (int i = 0; i < sun_count; i++)
	{
		std::string sun_position_uniform("suns[" + std::to_string(i) + "].position");
		std::string sun_illuminance_uniform("suns[" + std::to_string(i) + "].illuminance");

		Vec3 light_position = frame.m_sunlight_positions[i];
		ocean_surface_prgm->setUniform(sun_position_uniform.c_str(), light_position);

		// Compute luminance of sun (simplified to a pointlight) just before it hits the atmosphere
		float sun_luminous_power = GCoreComponents::sunlightManager().getLumen(i);

		//TODO compute actual illuminance

		ocean_surface_prgm->setUniform(sun_illuminance_uniform.c_str(), 100000.0f);
	}

	ocean_surface_prgm->setUniform("num_suns", sun_count);

	ocean_surface_prgm->setUniform("size", m_data[0].ocean_patch_size);
	ocean_surface_prgm->setUniform("grid_size", m_data[0].grid_size);

	ocean_surface_mesh->draw();
}

void OceanComponentManager::createGpuResources()
{
	twiddle_precompute_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_twiddle_precompute_c.glsl" }, "ocean_twiddle_precompute").resource;

	intial_spectrum_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_initial_spectrum_c.glsl" }, "ocean_intial_spectrum").resource;

	spectrum_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_spectrum_c.glsl" }, "ocean_spectrum").resource;

	ifft_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_ifft_c.glsl" }, "ocean_ifft").resource;

	inversion_prgm = GEngineCore::resourceManager().createShaderProgram({"../resources/shaders/ocean/ocean_ifft_inversion_c.glsl"}, "ocean_inversion").resource;

	displacement_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_write_displacement_c.glsl" }, "ocean_displacement").resource;

	compute_normal_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_normal_c.glsl" }, "ocean_computeNormal").resource;

	ocean_surface_prgm = GEngineCore::resourceManager().createShaderProgram({ "../resources/shaders/ocean/ocean_surface_v.glsl", "../resources/shaders/ocean/ocean_surface_f.glsl" }, "ocean_surface").resource;

	std::vector<float> vertex_data;
	std::vector<uint> index_data;

	// Dummy surface mesh
	//static float vertices[24] = { -10.0,0.0,-10.0, 0.0,1.0,0.0, 0.0,0.0,
	//								10.0,0.0,-10.0, 0.0,1.0,0.0, 0.0,0.0,
	//								0.0,0.0,10.0, 0.0,1.0,0.0, 0.0,0.0 };

	// TODO: make corresponding to grid resolution

	int subdivs_x = 512;
	int subdivs_y = 512;

	std::vector<float> vertices((subdivs_x+1) * (subdivs_y +1) * 5);

	float offsetX = -(1.0f / 2.0f);
	float offsetY = -(1.0f / 2.0f);
	float patchSizeX = 1.0f / subdivs_x;
	float patchSizeY = 1.0f / subdivs_y;

	for (int i = 0; i <= subdivs_y; i++)
	{
		for (int j = 0; j <= subdivs_x; j++)
		{
			vertices[(i * (subdivs_x+1) + j)*5 + 0] = (offsetX + float(j) * patchSizeX); //x
			vertices[(i * (subdivs_x+1) + j)*5 + 1] = (0.0); //y
			vertices[(i * (subdivs_x+1) + j)*5 + 2] = (offsetY + float(i) * patchSizeY); //z
			vertices[(i * (subdivs_x+1) + j)*5 + 3] = ((float)j / subdivs_x); //u
			vertices[(i * (subdivs_x+1) + j)*5 + 4] = ((float)i / subdivs_y); //v
		}
	}

	std::vector<uint8_t> surface_vertices(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + ((subdivs_x + 1) * (subdivs_y + 1) * 5 * 4));

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
			int c = j + i*(subdivs_x+1);

			// add indices for one patch
			surface_indices.push_back(c);
			surface_indices.push_back(c + n);
			surface_indices.push_back(c + e);

			surface_indices.push_back(c + n);
			surface_indices.push_back(c + ne);
			surface_indices.push_back(c + e);
		}
	}

	VertexLayout vertex_desc(5 * 4, { VertexLayout::Attribute(GL_FLOAT,3,GL_FALSE,0),VertexLayout::Attribute(GL_FLOAT,2,GL_FALSE,sizeof(GLfloat) * 3) });
	ocean_surface_mesh = GEngineCore::resourceManager().createMesh("ocean_surface_mesh", surface_vertices, surface_indices, vertex_desc, GL_TRIANGLES).resource;
}

void OceanComponentManager::precomputeFresnel()
{
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

	TextureLayout fresnel_lut_layout(GL_R32F, 512, 1, 1, GL_RED, GL_FLOAT, 1,
		{ std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT),
			std::pair<GLenum,GLenum>(GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT) }, {});
	ocean_fresnel_lut = GEngineCore::resourceManager().createTexture2D("ocean_fresnel_lut",fresnel_lut_layout,fresnel_data.data()).resource;
}

void OceanComponentManager::createComponentGpuResources(uint index)
{
	// for now create random texture on the CPU
	std::random_device rd;
	std::mt19937 mt(rd());
	std::normal_distribution<float> gaussian_dist(0.0f, 1.0f);
	std::vector<float> gaussian_noise(m_data[index].grid_size * m_data[index].grid_size * 4, 1.0f);
	for (int i=0; i< gaussian_noise.size(); i++)
	{
		gaussian_noise[i] = gaussian_dist(mt);
	}

	TextureLayout rgba32f_layout(GL_RGBA32F, m_data[index].grid_size, m_data[index].grid_size, 1, GL_RGBA, GL_FLOAT, 1);
	TextureLayout rg32f_layout(GL_RG32F, m_data[index].grid_size, m_data[index].grid_size, 1, GL_RG, GL_FLOAT, 1);

	m_data[index].gaussian_noise = GEngineCore::resourceManager().createTexture2D("ocean_gaussian_noise_" + std::to_string(index), rgba32f_layout, gaussian_noise.data()).resource;
	m_data[index].tilde_h0_of_k = GEngineCore::resourceManager().createTexture2D("ocean_h0_k_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].tilde_h0_of_minus_k = GEngineCore::resourceManager().createTexture2D("ocean_h0_-k_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].spectrum_x_displacement = GEngineCore::resourceManager().createTexture2D("ocean_spectrum_x_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].spectrum_y_displacement = GEngineCore::resourceManager().createTexture2D("ocean_spectrum_y_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].spectrum_z_displacement = GEngineCore::resourceManager().createTexture2D("ocean_spectrum_z_" + std::to_string(index), rg32f_layout, NULL).resource;
	
	m_data[index].twiddle = GEngineCore::resourceManager().createTexture2D("ocean_twiddle_" + std::to_string(index), rgba32f_layout, NULL).resource;
	m_data[index].ifft_x_a = GEngineCore::resourceManager().createTexture2D("ocean_ifft_x_a_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].ifft_x_b = GEngineCore::resourceManager().createTexture2D("ocean_ifft_x_b_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].ifft_y_a = GEngineCore::resourceManager().createTexture2D("ocean_ifft_y_a_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].ifft_y_b = GEngineCore::resourceManager().createTexture2D("ocean_ifft_y_b_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].ifft_z_a = GEngineCore::resourceManager().createTexture2D("ocean_ifft_z_a_" + std::to_string(index), rg32f_layout, NULL).resource;
	m_data[index].ifft_z_b = GEngineCore::resourceManager().createTexture2D("ocean_ifft_z_b_" + std::to_string(index), rg32f_layout, NULL).resource;
	
	m_data[index].displacement = GEngineCore::resourceManager().createTexture2D("ocean_displacement_" + std::to_string(index), rgba32f_layout, NULL).resource;
	m_data[index].normal = GEngineCore::resourceManager().createTexture2D("ocean_normal_" + std::to_string(index), rgba32f_layout, NULL).resource;
}

void OceanComponentManager::computeIntialSpectrum(uint index)
{
	intial_spectrum_prgm->use();

	glActiveTexture(GL_TEXTURE0);
	m_data[index].gaussian_noise->bindTexture();
	intial_spectrum_prgm->setUniform("gaussian_noise_tx2D", 0);

	glBindImageTexture(0, m_data[index].tilde_h0_of_k->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	intial_spectrum_prgm->setUniform("tilde_h0_of_k_tx2D", 0);

	glBindImageTexture(1, m_data[index].tilde_h0_of_minus_k->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	intial_spectrum_prgm->setUniform("tilde_h0_of_minus_k_tx2D", 1);

	// TODO pass an actual wind value
	intial_spectrum_prgm->setUniform("wind", Vec2(500.0f,500.0f));
	intial_spectrum_prgm->setUniform("size", Vec2(m_data[index].ocean_patch_size));
	intial_spectrum_prgm->setUniform("A", m_data[index].ocean_wave_height);
	intial_spectrum_prgm->setUniform("grid_size", m_data[index].grid_size);

	intial_spectrum_prgm->dispatchCompute(m_data[index].grid_size, m_data[index].grid_size, 1);

	// As the last necessary precomputation, it adds the perFrame updates and rendering
	GEngineCore::renderingPipeline().addPerFrameGpuTask(std::bind(&OceanComponentManager::drawOceanSurfaces, this, std::placeholders::_1), DeferredRenderingPipeline::RenderPass::OCEAN_PASS);
	GEngineCore::renderingPipeline().addPerFrameComputeGpuTask(std::bind(&OceanComponentManager::updateOceanHeightfields, this));
}

void OceanComponentManager::computeTwiddleTexture(uint index)
{
	// Bit reversed order of stage 0 indices
	std::vector<int> bit_reversed_indices(m_data[index].grid_size);
	for (int i = 0; i < bit_reversed_indices.size(); i++)
	{
		//CAUTION! This is for 9bit indices
		int b = 0;

		if (m_data[index].grid_size == 512)
		{
			b = ((i & 0x1) != 0) ? b | (0x100) : b;
			b = ((i & 0x2) != 0) ? b | (0x80) : b;
			b = ((i & 0x4) != 0) ? b | (0x40) : b;
			b = ((i & 0x8) != 0) ? b | (0x20) : b;
			b = ((i & 0x10) != 0) ? b | (0x10) : b;
			b = ((i & 0x20) != 0) ? b | (0x8) : b;
			b = ((i & 0x40) != 0) ? b | (0x4) : b;
			b = ((i & 0x80) != 0) ? b | (0x2) : b;
			b = ((i & 0x100) != 0) ? b | (0x1) : b;
		}
		else if (m_data[index].grid_size == 256)
		{
			b = ((i & 0x1) != 0) ? b | (0x80) : b;
			b = ((i & 0x2) != 0) ? b | (0x40) : b;
			b = ((i & 0x4) != 0) ? b | (0x20) : b;
			b = ((i & 0x8) != 0) ? b | (0x10) : b;
			b = ((i & 0x10) != 0) ? b | (0x8) : b;
			b = ((i & 0x20) != 0) ? b | (0x4) : b;
			b = ((i & 0x40) != 0) ? b | (0x2) : b;
			b = ((i & 0x80) != 0) ? b | (0x1) : b;
		}

		bit_reversed_indices[i] = b;

		//std::cout << "Bit reversed indices: " << b << std::endl;
	}

	ShaderStorageBufferObject indices_ssbo(bit_reversed_indices);

	twiddle_precompute_prgm->use();

	glBindImageTexture(0, m_data[index].twiddle->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	twiddle_precompute_prgm->setUniform("twiddle_tx2D", 0);
	twiddle_precompute_prgm->setUniform("grid_size", m_data[index].grid_size);

	indices_ssbo.bind(0);

	twiddle_precompute_prgm->dispatchCompute( static_cast<GLuint>(std::log2(m_data[index].grid_size)), m_data[index].grid_size/2, 1);
}

void OceanComponentManager::updateSpectrum(uint index, double t)
{
	spectrum_prgm->use();

	glActiveTexture(GL_TEXTURE0);
	m_data[index].tilde_h0_of_k->bindTexture();
	spectrum_prgm->setUniform("tilde_h0_of_k_tx2D", 0);

	glActiveTexture(GL_TEXTURE1);
	m_data[index].tilde_h0_of_minus_k->bindTexture();
	spectrum_prgm->setUniform("tilde_h0_of_minus_k_tx2D", 1);

	glBindImageTexture(0, m_data[index].spectrum_x_displacement->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
	spectrum_prgm->setUniform("spectrum_x_tx2D", 0);
	glBindImageTexture(1, m_data[index].spectrum_y_displacement->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
	spectrum_prgm->setUniform("spectrum_y_tx2D", 1);
	glBindImageTexture(2, m_data[index].spectrum_z_displacement->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG32F);
	spectrum_prgm->setUniform("spectrum_z_tx2D", 2);

	spectrum_prgm->setUniform("T", 30.0f);
	spectrum_prgm->setUniform("t", (float)t);
	spectrum_prgm->setUniform("grid_size", m_data[index].grid_size);
	spectrum_prgm->setUniform("size", Vec2(m_data[index].ocean_patch_size));

	spectrum_prgm->dispatchCompute(m_data[index].grid_size/4, m_data[index].grid_size/4, 1);
}

void OceanComponentManager::computeIFFT(uint index)
{
	ifft_prgm->use();

	glActiveTexture(GL_TEXTURE0);
	m_data[index].twiddle->bindTexture();
	ifft_prgm->setUniform("twiddle_tx2D", 0);

	// first ifft stage of first direction - read data from spectra and writ to ifft_*_a

	ifft_prgm->setUniform("src_x_tx2D", 0);
	ifft_prgm->setUniform("src_y_tx2D", 1);
	ifft_prgm->setUniform("src_z_tx2D", 2);

	ifft_prgm->setUniform("tgt_x_tx2D", 3);
	ifft_prgm->setUniform("tgt_y_tx2D", 4);
	ifft_prgm->setUniform("tgt_z_tx2D", 5);

	glBindImageTexture(0, m_data[index].spectrum_x_displacement->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(1, m_data[index].spectrum_y_displacement->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(2, m_data[index].spectrum_z_displacement->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	
	glBindImageTexture(3, m_data[index].ifft_x_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	glBindImageTexture(4, m_data[index].ifft_y_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
	glBindImageTexture(5, m_data[index].ifft_z_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

	ifft_prgm->setUniform("ifft_stage", 0);
	ifft_prgm->setUniform("ifft_direction", 0);

	ifft_prgm->dispatchCompute(m_data[index].grid_size, m_data[index].grid_size/2, 1);

	int src = 0;

	//ifft_prgm->setUniform("ifft_direction", 0);

	for (int j = 1; j < ( std::log2(m_data[index].grid_size)); j++)
	{
		// calling glBindImage expensive? if-query in shader faster?
		if (src == 0)
		{
			glBindImageTexture(0, m_data[index].ifft_x_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(1, m_data[index].ifft_y_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(2, m_data[index].ifft_z_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

			glBindImageTexture(3, m_data[index].ifft_x_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(4, m_data[index].ifft_y_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(5, m_data[index].ifft_z_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		}
		else
		{
			glBindImageTexture(0, m_data[index].ifft_x_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(1, m_data[index].ifft_y_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(2, m_data[index].ifft_z_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

			glBindImageTexture(3, m_data[index].ifft_x_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(4, m_data[index].ifft_y_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(5, m_data[index].ifft_z_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		}

		ifft_prgm->setUniform("ifft_stage", j );

		ifft_prgm->dispatchCompute(m_data[index].grid_size/4, m_data[index].grid_size / (2 * 4), 1);

		src = (src == 0) ? 1 : 0;

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	

	ifft_prgm->setUniform("ifft_direction", 1);

	for (int j = 0; j < (std::log2(m_data[index].grid_size)); j++)
	{
		// calling glBindImage expensive? if-query in shader faster?
		if (src == 0)
		{
			glBindImageTexture(0, m_data[index].ifft_x_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(1, m_data[index].ifft_y_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(2, m_data[index].ifft_z_a->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

			glBindImageTexture(3, m_data[index].ifft_x_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(4, m_data[index].ifft_y_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(5, m_data[index].ifft_z_b->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		}
		else
		{
			glBindImageTexture(0, m_data[index].ifft_x_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(1, m_data[index].ifft_y_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
			glBindImageTexture(2, m_data[index].ifft_z_b->getName(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);

			glBindImageTexture(3, m_data[index].ifft_x_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(4, m_data[index].ifft_y_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
			glBindImageTexture(5, m_data[index].ifft_z_a->getName(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		}

		ifft_prgm->setUniform("ifft_stage", j);

		ifft_prgm->dispatchCompute(m_data[index].grid_size/4, m_data[index].grid_size/ (2*4), 1);

		src = (src == 0) ? 1 : 0;

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	
}

void OceanComponentManager::updateDisplacementNormal(uint index)
{
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

	glBindImageTexture(0, m_data[index].ifft_x_a->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(1, m_data[index].ifft_y_a->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(2, m_data[index].ifft_z_a->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(3, m_data[index].ifft_x_b->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(4, m_data[index].ifft_y_b->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(5, m_data[index].ifft_z_b->getName(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG32F);

	displacement_prgm->setUniform("ifft_x_tx2D", 0);
	displacement_prgm->setUniform("ifft_y_tx2D", 1);
	displacement_prgm->setUniform("ifft_z_tx2D", 2);

	displacement_prgm->setUniform("ifft_x_b_tx2D", 3);
	displacement_prgm->setUniform("ifft_y_b_tx2D", 4);
	displacement_prgm->setUniform("ifft_z_b_tx2D", 5);

	glBindImageTexture(6, m_data[index].displacement->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	displacement_prgm->setUniform("displacement_tx2D", 6);

	displacement_prgm->setUniform("grid_size", m_data[index].grid_size);

	displacement_prgm->dispatchCompute(m_data[index].grid_size/4, m_data[index].grid_size/4,1);


	compute_normal_prgm->use();

	glActiveTexture(GL_TEXTURE0);
	m_data[index].displacement->bindTexture();
	compute_normal_prgm->setUniform("displacemnet_tx2D", 0);

	glBindImageTexture(0, m_data[index].normal->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	compute_normal_prgm->setUniform("normal_tx2D", 0);

	compute_normal_prgm->dispatchCompute(m_data[index].grid_size/4, m_data[index].grid_size/4, 1);
}