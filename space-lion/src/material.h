#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"

struct materialInfo
{
	materialInfo() : id(0) {}
	int id;
	char* diff_path;
	char* spec_path;
	char* normal_path;
};

class material
{
private:
	int id;

	GLSLProgram *shaderProgram;

	texture *diffuseMap;
	texture *specularMap;
	texture *normalMap;

public:
	~material();

	material(int,GLSLProgram*,texture*,texture*,texture*);
	
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool update(int,GLSLProgram*,texture*,texture*,texture*);

	int getId() {return id;}
	GLSLProgram* getShaderProgram() {return shaderProgram;}
	const texture* getDiffuseMap() {return diffuseMap;}
	const texture* getSpecularMap() {return specularMap;}
	const texture* getNormalMap() {return normalMap;}
};

#endif
