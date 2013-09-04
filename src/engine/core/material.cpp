#include "material.h"


Material::~Material(void)
{
}

Material::Material(int in_id,std::shared_ptr<GLSLProgram> prgm,std::shared_ptr<Texture> diff,
					std::shared_ptr<Texture> spec,std::shared_ptr<Texture> roughness,std::shared_ptr<Texture> normal) :
	id(in_id), shaderProgram(prgm), diffuseMap(diff), specularMap(spec), roughnessMap(roughness), normalMap(normal)
{
}


void Material::use()
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderProgram->setUniform("diffuse_tx2D",0);
	diffuseMap->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	shaderProgram->setUniform("specular_tx2D",1);
	specularMap->bindTexture();
	glActiveTexture(GL_TEXTURE2);
	shaderProgram->setUniform("roughness_tx2D",2);
	roughnessMap->bindTexture();
	glActiveTexture(GL_TEXTURE3);
	shaderProgram->setUniform("normal_tx2D",3);
	normalMap->bindTexture();
}