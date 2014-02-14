#ifndef atmosphere_h
#define atmosphere_h

#include <memory>

#include "../engine/core/texture.h"
#include "../engine/core/resourceManager.h"
#include "../engine/core/postProcessor.h"
#include "../engine/core/sceneCamera.h"

class Atmosphere
{
private:
	/**
	* Textures storing the precomputed atmospheric scattering results
	*/
	std::shared_ptr<Texture2D> m_transmittance_table;
	std::shared_ptr<Texture3D> m_rayleigh_inscatter_table;
	std::shared_ptr<Texture3D> m_mie_inscatter_table;
	std::shared_ptr<Texture2D> m_irradiance_table;

	/**
	* GLSL (compute)shader programs for precomputing atmospheric scattering tables
	*/
	std::shared_ptr<GLSLProgram> m_transmittance_prgm;
	std::shared_ptr<GLSLProgram> m_inscatter_single_prgm;
	std::shared_ptr<GLSLProgram> m_inscatter_multipleA_prgm;
	std::shared_ptr<GLSLProgram> m_inscatter_multipleB_prgm;
	std::shared_ptr<GLSLProgram> m_copy_inscatter_single_prgm;
	std::shared_ptr<GLSLProgram> m_copy_inscatter_multiple_prgm;
	std::shared_ptr<GLSLProgram> m_irradiance_single_prgm;
	std::shared_ptr<GLSLProgram> m_irradiance_multiple_prgm;
	std::shared_ptr<GLSLProgram> m_copy_irradiance_prgm;

	/**
	* GLSL shader for rendering atmosphere
	*/
	std::shared_ptr<GLSLProgram> m_sky_prgm;

	/**
	* Screen-filling quad for rendering
	*/
	Mesh m_render_plane;

	/**
	* Atmosphere attributes
	*/
	GLfloat m_min_altitide;
	GLfloat m_max_altitude;
	GLfloat center;

public:
	Atmosphere();
	Atmosphere(GLfloat min_alt, GLfloat max_alt);
	~Atmosphere();

	bool init(ResourceManager* resourceMngr);

	void precomputeTransmittance();
	void precomputeInscatterSingle();
	void precomputeIrradianceSingle();

	void render(PostProcessor* post_proc, SceneCamera * const camera_ptr, float time_of_day, FramebufferObject* terrain_fbo);
};

#endif