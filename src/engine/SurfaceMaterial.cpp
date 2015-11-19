#include "SurfaceMaterial.hpp"


SurfaceMaterial::~SurfaceMaterial(void)
{
}

SurfaceMaterial::SurfaceMaterial(std::string name,std::shared_ptr<GLSLProgram> prgm,std::shared_ptr<Texture> diff,
					std::shared_ptr<Texture> spec,std::shared_ptr<Texture> roughness,std::shared_ptr<Texture> normal) :
	Material(name, prgm), m_diffuseMap(diff), m_specularMap(spec), m_roughnessMap(roughness), m_normalMap(normal)
{
}


void SurfaceMaterial::use()
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	m_shader_prgm->setUniform("diffuse_tx2D",0);
	m_diffuseMap->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	m_shader_prgm->setUniform("specular_tx2D",1);
	m_specularMap->bindTexture();
	glActiveTexture(GL_TEXTURE2);
	m_shader_prgm->setUniform("roughness_tx2D",2);
	m_roughnessMap->bindTexture();
	glActiveTexture(GL_TEXTURE3);
	m_shader_prgm->setUniform("normal_tx2D",3);
	m_normalMap->bindTexture();
}