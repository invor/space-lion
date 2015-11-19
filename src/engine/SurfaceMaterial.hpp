#ifndef SurfaceMaterial_hpp
#define SurfaceMaterial_hpp

#include "Material.hpp"

struct SurfaceMaterialInfo
{
	SurfaceMaterialInfo() {}
	std::string vs_path;
	std::string fs_path;
	std::string geo_path;
	std::string tessCtrl_path;
	std::string tessEval_path;

	std::string diff_path;
	std::string spec_path;
	std::string roughness_path;
	std::string normal_path;
};

class SurfaceMaterial : public Material
{
private:
	//TODO rethink material parameters
	std::shared_ptr<Texture> m_diffuseMap;
	std::shared_ptr<Texture> m_specularMap;
	std::shared_ptr<Texture> m_roughnessMap;
	std::shared_ptr<Texture> m_normalMap;

public:
	SurfaceMaterial(std::string,std::shared_ptr<GLSLProgram>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>,std::shared_ptr<Texture>);
	~SurfaceMaterial();
	
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool update(int,GLSLProgram*,Texture*,Texture*,Texture*,Texture*);

	void use();

	std::shared_ptr<Texture> getDiffuseMap() {return m_diffuseMap;}
	std::shared_ptr<Texture> getSpecularMap() {return m_specularMap;}
	std::shared_ptr<Texture> getNormalMap() {return m_normalMap;}
};

#endif
