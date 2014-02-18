#include "planetaryScene.h"

PlanetaryScene::PlanetaryScene()
{
}

PlanetaryScene::PlanetaryScene(glm::vec3 beta_r, glm::vec3 beta_m, GLfloat h_r, GLfloat h_m, GLfloat min_alt, GLfloat max_alt, glm::vec3 center)
	: m_sky(beta_r,beta_m,h_r,h_m,min_alt,max_alt,center)
{
}

PlanetaryScene::~PlanetaryScene()
{
}


bool PlanetaryScene::initAtmosphere(ResourceManager* resourceMngr)
{
	return m_sky.init(resourceMngr);
}

bool PlanetaryScene::loadTerrain(int size, float range, std::shared_ptr<Material> material, std::shared_ptr<Texture> heightmap)
{
	m_terrain.setSize(size);
	m_terrain.setRange(range);
	if(!(m_terrain.init(material, heightmap)))
		return false;
	return true;
}

void PlanetaryScene::renderTerrain()
{
	m_terrain.getMaterial()->getShaderProgram()->use();

	int light_counter = 0;
	m_terrain.getMaterial()->getShaderProgram()->setUniform("lights.position", lightSourceList.begin()->getPosition());
	m_terrain.getMaterial()->getShaderProgram()->setUniform("lights.intensity", lightSourceList.begin()->getColour());
	m_terrain.getMaterial()->getShaderProgram()->setUniform("num_lights", light_counter);

	glm::mat4 view_mx = activeCamera->computeViewMatrix();
	glm::mat4 projection_mx = activeCamera->computeProjectionMatrix(0.001f, 100000.0f);

	m_terrain.getMaterial()->getShaderProgram()->setUniform("model_view_matrix", view_mx);
	m_terrain.getMaterial()->getShaderProgram()->setUniform("view_matrix", view_mx);
	m_terrain.getMaterial()->getShaderProgram()->setUniform("projection_matrix", projection_mx);

	m_terrain.render();
}

void PlanetaryScene::renderSky(float time_of_day, FramebufferObject* terrain_fbo)
{
	m_sky.render(activeCamera,time_of_day, terrain_fbo);
}