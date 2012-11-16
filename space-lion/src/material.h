#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"

class material
{
private:
	int id;

	const GLSLProgram *shaderProgram;

	const texture *diffuseMap;
	const texture *specularMap;
	const texture *normalMap;
	const texture *alphaMap;

	float opacity;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
public:
	material(void);
	~material(void);
	
	int getId();
	void setShaderProgram(const GLSLProgram *shrPtr);
	void setDiffuseMap(const texture *texPtr);
	void setSpecularMap(const texture *texPtr);
	void setNormalMap(const texture *texPtr);
	void setAlphaMap(const texture *texPtr);

	void setOpacity(float op);
	void setDiffuseColor(glm::vec3 diffColor);
	void setSpecularColor(glm::vec3 specColor);
};

#endif