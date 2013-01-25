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
	
	const int getId() {return id;}
	GLSLProgram * const getShaderProgram() {return shaderProgram;}
	texture * const getDiffuseMap() {return diffuseMap;}
	texture * const getSpecularMap() {return specularMap;}
	texture * const getNormalMap() {return normalMap;}
	texture * const getAlphaMap() {return alphaMap;}

	const float getOpacity() {return opacity;}
	const glm::vec3 getDiffuseColor() {return diffuseColor;}
};

#endif