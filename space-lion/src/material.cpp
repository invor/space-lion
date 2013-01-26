#include "material.h"


material::~material(void)
{
}

material::material(int in_id,GLSLProgram& prgm,texture& diff,texture& spec,texture& normal,texture& alpha,float op,glm::vec3 diffColor) :
	id(in_id), shaderProgram(prgm), diffuseMap(diff), specularMap(spec), normalMap(normal), alphaMap(alpha), opacity(op), diffuseColor(diffColor)
{
}