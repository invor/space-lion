#include "material.h"


material::material() : id(), shaderProgram(), diffuseMap(), specularMap(), normalMap(), alphaMap(), opacity(), diffuseColor()
{
}


material::~material(void)
{
}

material::material(int in_id,GLSLProgram*prgm,texture*diff,texture*spec,texture*normal,texture*alpha,float op,glm::vec3 diffColor) :
	id(in_id), shaderProgram(prgm), diffuseMap(diff), specularMap(spec), normalMap(normal), alphaMap(alpha), opacity(op), diffuseColor(diffColor)
{
}

const int material::getId()
{
	return id;
}

GLSLProgram * const material::getShaderProgram()
{
	return shaderProgram;
}

texture * const material::getDiffuseMap()
{
	return diffuseMap;
}

texture * const material::getSpecularMap()
{
	return specularMap;
}

texture * const material::getNormalMap()
{
	return normalMap;
}

texture * const material::getAlphaMap()
{
	return alphaMap;
}

const float material::getOpacity()
{
	return opacity;
}

const glm::vec3 material::getDiffuseColor()
{
	return diffuseColor;
}