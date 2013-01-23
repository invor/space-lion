#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"

class material
{
private:
	const int id;

	GLSLProgram * const shaderProgram;

	texture * const diffuseMap;
	texture * const specularMap;
	texture * const normalMap;
	texture * const alphaMap;

	const float opacity;
	const glm::vec3 diffuseColor;
public:
	material(void);
	~material(void);

	material(int,GLSLProgram*,texture *,texture *,texture *,texture *,float,glm::vec3);
	
	const int getId();
	GLSLProgram * const getShaderProgram();
	texture * const getDiffuseMap();
	texture * const getSpecularMap();
	texture * const getNormalMap();
	texture * const getAlphaMap();

	const float getOpacity();
	const glm::vec3 getDiffuseColor();
};

#endif