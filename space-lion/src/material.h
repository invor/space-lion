#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"

struct MaterialInfo
{
	MaterialInfo() : id(0) {}
	int id;
	char* diff_path;
	char* spec_path;
	char* normal_path;
};

class Material
{
private:
	int id;

	GLSLProgram *shaderProgram;

	Texture *diffuseMap;
	Texture *specularMap;
	Texture *normalMap;

public:
	~Material();

	Material(int,GLSLProgram*,Texture*,Texture*,Texture*);
	
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool update(int,GLSLProgram*,Texture*,Texture*,Texture*);

	void use();

	int getId() {return id;}
	GLSLProgram* getShaderProgram() {return shaderProgram;}
	const Texture* getDiffuseMap() {return diffuseMap;}
	const Texture* getSpecularMap() {return specularMap;}
	const Texture* getNormalMap() {return normalMap;}
};

#endif
