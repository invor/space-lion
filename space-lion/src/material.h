#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"

class material
{
private:
	const int id;

	GLSLProgram& shaderProgram;

	texture& diffuseMap;
	texture& specularMap;
	texture& normalMap;
	texture& alphaMap;

	const float opacity;
	const glm::vec3 diffuseColor;
public:
	~material();

	material(int,GLSLProgram&,texture&,texture&,texture&,texture&,float,glm::vec3);
	
	const int getId() {return id;}
	GLSLProgram& getShaderProgram() {return shaderProgram;}
	texture& getDiffuseMap() {return diffuseMap;}
	texture& getSpecularMap() {return specularMap;}
	texture& getNormalMap() {return normalMap;}
	texture& getAlphaMap() {return alphaMap;}

	const float getOpacity() {return opacity;}
	const glm::vec3 getDiffuseColor() {return diffuseColor;}
};

#endif