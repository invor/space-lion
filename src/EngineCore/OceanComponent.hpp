#ifndef OceanComponent_hpp
#define OceanComponent_hpp

class Texture2D;
class Mesh;
class GLSLProgram;

struct Frame;

#include "EntityManager.hpp"

// std includes
#include <unordered_map>
#include <random>

class OceanComponentManager
{
private:

	struct Data
	{
		Data(Entity e, float ocean_wave_height, float ocean_patch_size, uint grid_size)
			: entity(e), ocean_wave_height(ocean_wave_height), ocean_patch_size(ocean_patch_size), grid_size(grid_size) {}

		//	uint used;						///< number of components currently in use
		//	uint allocated;					///< number of components that the allocated memery can hold
		//	void* buffer;					///< raw data pointer

		Entity entity;					///< entity owning that owns the component
		float ocean_wave_height;
		float ocean_patch_size;
		uint grid_size;

		Texture2D* gaussian_noise;
		Texture2D* tilde_h0_of_k;
		Texture2D* tilde_h0_of_minus_k;
		Texture2D* spectrum_x_displacement;
		Texture2D* spectrum_y_displacement;
		Texture2D* spectrum_z_displacement;

		Texture2D* twiddle;
		Texture2D* ifft_x_a;
		Texture2D* ifft_x_b;
		Texture2D* ifft_y_a;
		Texture2D* ifft_y_b;
		Texture2D* ifft_z_a;
		Texture2D* ifft_z_b;

		Texture2D* displacement;
		Texture2D* normal;	
	};

	std::vector<Data> m_data;

	GLSLProgram* twiddle_precompute_prgm;
	GLSLProgram* intial_spectrum_prgm;
	GLSLProgram* spectrum_prgm;
	GLSLProgram* ifft_prgm;
	GLSLProgram* inversion_prgm;
	GLSLProgram* displacement_prgm;
	GLSLProgram* compute_normal_prgm;

	GLSLProgram* ocean_surface_prgm;
	Mesh* ocean_surface_mesh;
	Texture2D* ocean_fresnel_lut;

	std::unordered_map<uint, uint> m_index_map;


	std::chrono::time_point<std::chrono::high_resolution_clock> time;

	// GPU methods (Caution! Only call from GPU thread)
	void createGpuResources();

	void precomputeFresnel();

	void createComponentGpuResources(uint index);

	void computeIntialSpectrum(uint index);
	void computeTwiddleTexture(uint index);

	void updateSpectrum(uint index, double t);
	void computeIFFT(uint index);
	void updateDisplacementNormal(uint index);

public:
	OceanComponentManager();
	~OceanComponentManager();

	void addComponent(Entity entity, float wave_height, float patch_size, uint grid_size);

	std::pair<bool, uint> getIndex(uint entity_id);

	void updateOceanHeightfields();

	void drawOceanSurfaces(Frame const&);
};

#endif // !OceanComponent_hpp