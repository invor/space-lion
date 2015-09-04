#include "Material.h"


Material::~Material(void)
{
}

Material::Material(std::string name,std::shared_ptr<GLSLProgram> prgm,std::shared_ptr<Texture> diff,
					std::shared_ptr<Texture> spec,std::shared_ptr<Texture> roughness,std::shared_ptr<Texture> normal) :
	m_name(name), m_shaderProgram(prgm), m_diffuseMap(diff), m_specularMap(spec), m_roughnessMap(roughness), m_normalMap(normal)
{
}


void Material::use()
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	m_shaderProgram->setUniform("diffuse_tx2D",0);
	m_diffuseMap->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	m_shaderProgram->setUniform("specular_tx2D",1);
	m_specularMap->bindTexture();
	glActiveTexture(GL_TEXTURE2);
	m_shaderProgram->setUniform("roughness_tx2D",2);
	m_roughnessMap->bindTexture();
	glActiveTexture(GL_TEXTURE3);
	m_shaderProgram->setUniform("normal_tx2D",3);
	m_normalMap->bindTexture();
}