#include "material.h"


material::~material(void)
{
}

material::material(int in_id,GLSLProgram* prgm,texture* diff,texture* spec,texture* normal) :
	id(in_id), shaderProgram(prgm), diffuseMap(diff), specularMap(spec), normalMap(normal)
{
}
