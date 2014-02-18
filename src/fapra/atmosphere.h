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
	glm::vec3 m_beta_r;
	glm::vec3 m_beta_m;
	GLfloat m_h_r;
	GLfloat m_h_m;
	GLfloat m_min_altitude;
	GLfloat m_max_altitude;
	glm::vec3 m_center;

public:
	Atmosphere();
	Atmosphere(glm::vec3 beta_r, glm::vec3 beta_m, GLfloat h_r, GLfloat h_m, GLfloat min_alt, GLfloat max_alt, glm::vec3 center);
	~Atmosphere();

	/**
	* \brief initialize resources used for rendering the atmosphere
	* Precomutation of look up tables is performaned as part of the initialization
	* \param resourceMngr Pointer to the ResourceManger where some of the resources will be stored and managed.
	* \return Returns true if all resources could be created, false otherwise
	*/
	bool init(ResourceManager* resourceMngr);

	/**
	* \brief Precompute tables for transmittance
	*/
	void precomputeTransmittance();
	/**
	* \brief Precompute table for inscattering due to a single scattering event
	*/
	void precomputeInscatterSingle();
	/**
	* \brief Precompute table for irradiance due to a single scattering event
	* \note Currently not in use
	*/
	void precomputeIrradianceSingle();

	/**
	* \brief Render the sky together with a scene, that has been rendered in a previous pass
	* \param camera_ptr Pointer the active camera used for rendering.
	* \param time_of_day Time of day in minutes. Values should be withing 0 to 1440.
	* \param terrain_fbo Contains previously rendered scene. Basically a G-Buffer.
	*/
	void render(SceneCamera * const camera_ptr, float time_of_day, FramebufferObject* terrain_fbo);
};

#endif