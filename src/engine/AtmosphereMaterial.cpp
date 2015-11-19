#include "AtmosphereMaterial.hpp"

AtmosphereMaterial::AtmosphereMaterial( std::string name,
						std::shared_ptr<GLSLProgram> prgm,
						std::shared_ptr<Texture> transmittance_table,
						std::shared_ptr<Texture> rayleigh_inscatter_table,
						std::shared_ptr<Texture> mie_inscatter_table,
						std::shared_ptr<Texture> irradiance_table )
	: Material(name, prgm),
		m_transmittance_table(transmittance_table),
		m_rayleigh_inscatter_table(rayleigh_inscatter_table),
		m_mie_inscatter_table(mie_inscatter_table),
		m_irradiance_table(irradiance_table)
{

}

AtmosphereMaterial::~AtmosphereMaterial()
{
}

void AtmosphereMaterial::use()
{

}

std::shared_ptr<Texture> AtmosphereMaterial::getTransmittanceTable()
{
	return m_transmittance_table;
}

std::shared_ptr<Texture> AtmosphereMaterial::getRayleighInscatterTable()
{
	return m_rayleigh_inscatter_table;
}

std::shared_ptr<Texture> AtmosphereMaterial::getMieInscatterTable()
{
	return m_mie_inscatter_table;
}

std::shared_ptr<Texture> AtmosphereMaterial::getIrradianceTable()
{
	return m_irradiance_table;
}