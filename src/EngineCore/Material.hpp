#ifndef Material_hpp
#define Material_hpp

#include <string>
#include <memory>

#include "glowl.h"

struct MaterialInfo
{
	std::vector<std::string> shader_filepaths;

	std::vector<std::string> texture_filepaths;
};

class Material
{
public:
	Material(const std::string& name, const std::shared_ptr<GLSLProgram> prgm, const std::vector<std::shared_ptr<Texture>>& textures) 
		: m_name(name), m_shader_prgm(prgm), m_textures(textures) {};
	virtual ~Material() {};

	std::string getName(){ return m_name; }

	std::shared_ptr<GLSLProgram> getShaderProgram() { return m_shader_prgm; }

	std::vector<std::shared_ptr<Texture>>& getTextures() { return m_textures; }

protected:
	/** Name of the material. If material was loaded from a file, this should correspond to the file path */
	std::string m_name;

	/** Shader program used by the material */
	std::shared_ptr<GLSLProgram> m_shader_prgm;

	/** Texture maps used by the material */
	std::vector<std::shared_ptr<Texture>> m_textures;
};

#endif