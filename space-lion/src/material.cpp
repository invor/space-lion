#include "material.h"


material::material(void)
{
}


material::~material(void)
{
}

int material::getId()
{
	return id;
}

void material::setShaderProgram(const GLSLProgram *shrPtr)
{
	shaderProgram = shrPtr;
}

void material::setDiffuseMap(const texture *texPtr)
{
	diffuseMap = texPtr;
}

void material::setSpecularMap(const texture *texPtr)
{
	specularMap = texPtr;
}

void material::setNormalMap(const texture *texPtr)
{
	normalMap = texPtr;
}

void material::setAlphaMap(const texture *texPtr)
{
	alphaMap = texPtr;
}

void material::setOpacity(float op)
{
	opacity = op;
}

void material::setDiffuseColor(glm::vec3 diffColor)
{
	diffuseColor = diffColor;
}

void material::setSpecularColor(glm::vec3 specColor)
{
	specularColor = specColor;
}