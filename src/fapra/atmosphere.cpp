#include "atmosphere.h"

Atmosphere::Atmosphere()
{
}

Atmosphere::~Atmosphere()
{
}


bool Atmosphere::init(ResourceManager* resourceMngr)
{
	GLfloat data[256*64*3];
	for(int i=0; i<256*64*3; i++)
		data[i] = 1.0;

	/*	create all textures for storing the precomuted data */
	m_transmittance_table = std::shared_ptr<Texture2D>(new Texture2D());
	m_transmittance_table->load(GL_RGBA32F,256,64,GL_RGBA,GL_FLOAT,0);
	m_transmittance_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_transmittance_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	
	m_inscatter_table = std::shared_ptr<Texture3D>(new Texture3D());
	m_inscatter_table->load(GL_RGBA32F,256,128,32,GL_RGBA,GL_FLOAT,0);
	m_inscatter_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_inscatter_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	m_inscatter_table->texParameteri(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

	m_irradiance_table = std::shared_ptr<Texture2D>(new Texture2D());
	m_irradiance_table->load(GL_RGBA32F,64,16,GL_RGBA,GL_FLOAT,0);
	m_irradiance_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_irradiance_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	/*	create all precomputing shaders */	
	if( !resourceMngr->createShaderProgram(TRANSMITTANCE_COMPUTE,m_transmittance_prgm)) return false;
	if( !resourceMngr->createShaderProgram(INSCATTER_SINGLE,m_inscatter_single_prgm)) return false;
	if( !resourceMngr->createShaderProgram(IRRADIANCE_SINGLE,m_irradiance_single_prgm)) return false;

	precomputeTransmittance();

	return true;
}

void Atmosphere::precomputeTransmittance()
{
	m_transmittance_prgm->use();

	glBindImageTexture(0,m_transmittance_table->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	m_transmittance_prgm->setUniform("transmittance_tx2D",0);
	m_transmittance_prgm->setUniform("min_altitude",GLfloat(6360.0));
	m_transmittance_prgm->setUniform("max_altitude",GLfloat(6420.0));
	m_transmittance_prgm->setUniform("beta_r",glm::vec3(0.0058,0.0135,0.0331));
	m_transmittance_prgm->setUniform("beta_m",glm::vec3(0.00444,0.00444,0.00444));
	m_transmittance_prgm->setUniform("h_r",GLfloat(8.000));
	m_transmittance_prgm->setUniform("h_m",GLfloat(1.200));

	m_transmittance_prgm->dispatchCompute(256,64,1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


void Atmosphere::precomputeInscatterSingle()
{
	m_inscatter_single_prgm->use();
}

void Atmosphere::precomputeIrradianceSingle()
{
	m_irradiance_single_prgm->use();
}

void Atmosphere::render(PostProcessor* post_proc)
{
	post_proc->imageToFBO(m_transmittance_table->getHandle());
}