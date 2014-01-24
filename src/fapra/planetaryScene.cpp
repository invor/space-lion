#include "planetaryScene.h"

PlanetaryScene::PlanetaryScene()
{
}

PlanetaryScene::~PlanetaryScene()
{
}


void PlanetaryScene::initAtmosphere()
{
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
	glm::mat4 projection_mx = activeCamera->computeProjectionMatrix(1.0f, 500000.0f);

	m_terrain.getMaterial()->getShaderProgram()->setUniform("model_view_matrix", view_mx);
	m_terrain.getMaterial()->getShaderProgram()->setUniform("view_matrix", view_mx);
	m_terrain.getMaterial()->getShaderProgram()->setUniform("projection_matrix", projection_mx);

	m_terrain.render();
}

void PlanetaryScene::renderSky()
{
}