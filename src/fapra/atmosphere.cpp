#include "atmosphere.h"

Atmosphere::Atmosphere()
{
}

Atmosphere::~Atmosphere()
{
}


bool Atmosphere::init(ResourceManager* resourceMngr)
{
	/*	Create vertex geometry of the render plane */
	Vertex_pu *vertexArray = new Vertex_pu[4];
	GLuint *indexArray = new GLuint[6];

	vertexArray[0]=Vertex_pu(-1.0,-1.0,0.0,0.0,0.0);vertexArray[1]=Vertex_pu(-1.0,1.0,0.0,0.0,1.0);
	vertexArray[2]=Vertex_pu(1.0,1.0,0.0,1.0,1.0);vertexArray[3]=Vertex_pu(1.0,-1.0,0.0,1.0,0.0);
	
	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;

	if(!(m_render_plane.bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pu)*4,sizeof(GLuint)*6,GL_TRIANGLES))) return false;
	m_render_plane.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),0);
	m_render_plane.setVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),(GLvoid*) sizeof(Vertex_p));


	GLfloat data[256*64*3];
	for(int i=0; i<256*64*3; i++)
		data[i] = 1.0;

	/*	create all textures for storing the precomuted data */
	m_transmittance_table = std::shared_ptr<Texture2D>(new Texture2D());
	m_transmittance_table->load(GL_RGBA32F,256,64,GL_RGBA,GL_FLOAT,0);
	m_transmittance_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_transmittance_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	
	m_rayleigh_inscatter_table = std::shared_ptr<Texture3D>(new Texture3D());
	m_rayleigh_inscatter_table->load(GL_RGBA32F,256,128,32,GL_RGBA,GL_FLOAT,0);
	m_rayleigh_inscatter_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_rayleigh_inscatter_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	m_rayleigh_inscatter_table->texParameteri(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

	m_mie_inscatter_table = std::shared_ptr<Texture3D>(new Texture3D());
	m_mie_inscatter_table->load(GL_RGBA32F,256,128,32,GL_RGBA,GL_FLOAT,0);
	m_mie_inscatter_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_mie_inscatter_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	m_mie_inscatter_table->texParameteri(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

	m_irradiance_table = std::shared_ptr<Texture2D>(new Texture2D());
	m_irradiance_table->load(GL_RGBA32F,64,16,GL_RGBA,GL_FLOAT,0);
	m_irradiance_table->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	m_irradiance_table->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	/*	create all precomputing shaders */	
	if( !resourceMngr->createShaderProgram(TRANSMITTANCE_COMPUTE,m_transmittance_prgm)) return false;
	if( !resourceMngr->createShaderProgram(INSCATTER_SINGLE,m_inscatter_single_prgm)) return false;
	if( !resourceMngr->createShaderProgram(IRRADIANCE_SINGLE,m_irradiance_single_prgm)) return false;

	/*	create shader program for actual rendering */
	if( !resourceMngr->createShaderProgram(SKY,m_sky_prgm)) return false;

	precomputeTransmittance();
	precomputeInscatterSingle();

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

	glBindImageTexture(0,m_rayleigh_inscatter_table->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	m_inscatter_single_prgm->setUniform("rayleigh_inscatter_tx3D",0);

	glBindImageTexture(1,m_mie_inscatter_table->getHandle(),0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	m_inscatter_single_prgm->setUniform("mie_inscatter_tx3D",1);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	m_inscatter_single_prgm->setUniform("transmittance_tx2D",2);
	m_transmittance_table->bindTexture();
	glDisable(GL_TEXTURE_2D);

	m_inscatter_single_prgm->setUniform("min_altitude",GLfloat(6360.0));
	m_inscatter_single_prgm->setUniform("max_altitude",GLfloat(6420.0));
	m_inscatter_single_prgm->setUniform("beta_r",glm::vec3(0.0058,0.0135,0.0331));
	m_inscatter_single_prgm->setUniform("beta_m",glm::vec3(0.00444,0.00444,0.00444));
	m_inscatter_single_prgm->setUniform("h_r",GLfloat(8.0));
	m_inscatter_single_prgm->setUniform("h_m",GLfloat(1.2));

	m_inscatter_single_prgm->dispatchCompute(32,128,32);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Atmosphere::precomputeIrradianceSingle()
{
	m_irradiance_single_prgm->use();
}

void Atmosphere::render(PostProcessor* post_proc, SceneCamera * const camera_ptr, float time_of_day, FramebufferObject* terrain_fbo)
{
	//post_proc->imageToFBO(m_transmittance_table->getHandle());
	
	m_sky_prgm->use();
	
	glEnable(GL_TEXTURE_3D);
	glActiveTexture(GL_TEXTURE0);
	m_sky_prgm->setUniform("rayleigh_inscatter_tx3D",0);
	m_rayleigh_inscatter_table->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	m_sky_prgm->setUniform("mie_inscatter_tx3D",1);
	m_mie_inscatter_table->bindTexture();
	//reserve GL_TEXTURE2 for irradiance table
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	m_sky_prgm->setUniform("scene_diffuse_albedo_tx2D",3);
	terrain_fbo->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE4);
	m_sky_prgm->setUniform("scene_normal_tx2D",4);
	terrain_fbo->bindColorbuffer(1);
	glActiveTexture(GL_TEXTURE5);
	m_sky_prgm->setUniform("scene_tangent_bitangent_tx2D",5);
	terrain_fbo->bindColorbuffer(2);
	glActiveTexture(GL_TEXTURE6);
	m_sky_prgm->setUniform("scene_specular_roughness_tx2D",6);
	terrain_fbo->bindColorbuffer(3);
	glActiveTexture(GL_TEXTURE7);
	m_sky_prgm->setUniform("scene_linear_depth_tx2D",7);
	terrain_fbo->bindColorbuffer(4);
	glDisable(GL_TEXTURE_2D);
	
	m_sky_prgm->setUniform("view_mx", camera_ptr->computeViewMatrix());
	m_sky_prgm->setUniform("scene_depth_tx2D", 0);
	m_sky_prgm->setUniform("camera_position", camera_ptr->getPosition());
	m_sky_prgm->setUniform("fov_y", camera_ptr->getFieldOfView());
	m_sky_prgm->setUniform("aspect_ratio", camera_ptr->getAspectRatio());
	m_sky_prgm->setUniform("min_altitude",GLfloat(6360.0));
	m_sky_prgm->setUniform("max_altitude", GLfloat(6420.0));
	m_sky_prgm->setUniform("planet_center", glm::vec3(0.0,-6362.0,0.0));
	
	/*	crude computation of sun direction */
	/*	sun angle relative to a (0,1,0) zenith vector */
	float sun_angle = (time_of_day/1440.0) * 2.0 * 3.1415926535897932384626433832795 + 3.1415926535897932384626433832795;
	m_sky_prgm->setUniform("sun_direction", glm::vec3(0.0,cos(sun_angle),-sin(sun_angle)));
	
	m_render_plane.draw();
}