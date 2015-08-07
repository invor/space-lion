#ifndef material_h
#define material_h

#include "GLSLProgram.h"
#include "texture.h"
#include <memory>

struct MaterialInfo
{
	MaterialInfo() : id(0) {}
	int id;
	/*	shader type should be the index of a ShaderType enum */
	int shader_type;
	char* diff_path;
	char* spec_path;
	char* roughness_path;
	char* normal_path;
};

class Material
{
private:
	// Materials files are currently written by hand. Keeping track of used
	// id's is tedious and error prone. For now, use the path to the material
	// file as unique identifiert until the engine manages resources
	// automatically (including id assignment)
	std::string m_name;
	//int id;

	//TODO rethink material parameters
	std::shared_ptr<GLSLProgram> m_shaderProgram;

	std::shared_ptr<Texture> m_diffuseMap;
	std::shared_ptr<Texture> m_specularMap;
	std::shared_ptr<Texture> m_roughnessMap;
	std::shared_ptr<Texture> m_normalMap;

public:
	~Material();

	Material(std::string,std::shared_ptr<GLSLProgram>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>);
	
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool update(int,GLSLProgram*,Texture*,Texture*,Texture*,Texture*);

	void use();

	//int getId() {return id;}
	std::string getName() { return m_name; }
	std::shared_ptr<GLSLProgram> getShaderProgram() {return m_shaderProgram;}
	std::shared_ptr<Texture> getDiffuseMap() {return m_diffuseMap;}
	std::shared_ptr<Texture> getSpecularMap() {return m_specularMap;}
	std::shared_ptr<Texture> getNormalMap() {return m_normalMap;}
};

#endif
