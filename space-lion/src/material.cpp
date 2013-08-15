#include "material.h"


Material::~Material(void)
{
}

Material::Material(int in_id,GLSLProgram* prgm,Texture* diff,Texture* spec,Texture* normal) :
	id(in_id), shaderProgram(prgm), diffuseMap(diff), specularMap(spec), normalMap(normal)
{
}


void Material::use()
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderProgram->setUniform("diffuseMap",0);
	diffuseMap->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	shaderProgram->setUniform("specularMap",1);
	specularMap->bindTexture();
	glActiveTexture(GL_TEXTURE2);
	shaderProgram->setUniform("normalMap",2);
	normalMap->bindTexture();
}