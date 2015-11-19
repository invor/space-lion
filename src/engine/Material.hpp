#ifndef Material_hpp
#define Material_hpp

#include <string>
#include <memory>

#include "glowl.h"

class Material
{
public:
	Material(std::string name, std::shared_ptr<GLSLProgram> prgm) 
		: m_name(name), m_shader_prgm(prgm) {};
	virtual ~Material() {};

	virtual void use() = 0;

	std::string getName() { return m_name; }
	std::shared_ptr<GLSLProgram> getShaderProgram() {return m_shader_prgm;}

protected:
	// Materials files are currently written by hand. Keeping track of used
	// id's is tedious and error prone. For now, use the path to the material
	// file as unique identifiert until the engine manages resources
	// automatically (including id assignment)
	std::string m_name;

	std::shared_ptr<GLSLProgram> m_shader_prgm;
};

#endif