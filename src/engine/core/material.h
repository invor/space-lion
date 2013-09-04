#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"
#include <memory>

struct MaterialInfo
{
	MaterialInfo() : id(0) {}
	int id;
	char* diff_path;
	char* spec_path;
	char* roughness_path;
	char* normal_path;
};

class Material
{
private:
	int id;

	std::shared_ptr<GLSLProgram> shaderProgram;

	std::shared_ptr<Texture> diffuseMap;
	std::shared_ptr<Texture> specularMap;
	std::shared_ptr<Texture> roughnessMap;
	std::shared_ptr<Texture> normalMap;

public:
	~Material();

	Material(int,std::shared_ptr<GLSLProgram>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>);
	
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool update(int,GLSLProgram*,Texture*,Texture*,Texture*,Texture*);

	void use();

	int getId() {return id;}
	std::shared_ptr<GLSLProgram> getShaderProgram() {return shaderProgram;}
	std::shared_ptr<Texture> getDiffuseMap() {return diffuseMap;}
	std::shared_ptr<Texture> getSpecularMap() {return specularMap;}
	std::shared_ptr<Texture> getNormalMap() {return normalMap;}
};

#endif
