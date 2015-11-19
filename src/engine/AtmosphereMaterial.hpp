#ifndef AtmosphereMaterial_hpp
#define AtmosphereMaterial_hpp

#include "Material.hpp"

class AtmosphereMaterial : public Material
{
public:
	AtmosphereMaterial( std::string name,
						std::shared_ptr<GLSLProgram> prgm,
						std::shared_ptr<Texture> transmittance_table,
						std::shared_ptr<Texture> rayleigh_inscatter_table,
						std::shared_ptr<Texture> mie_inscatter_table,
						std::shared_ptr<Texture> irradiance_table );
	~AtmosphereMaterial();

	void use();

	std::shared_ptr<Texture> getTransmittanceTable();
	std::shared_ptr<Texture> getRayleighInscatterTable();
	std::shared_ptr<Texture> getMieInscatterTable();
	std::shared_ptr<Texture> getIrradianceTable();

private:
	std::shared_ptr<Texture> m_transmittance_table;
	std::shared_ptr<Texture> m_rayleigh_inscatter_table;
	std::shared_ptr<Texture> m_mie_inscatter_table;
	std::shared_ptr<Texture> m_irradiance_table;
};

#endif