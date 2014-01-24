#ifndef planetaryScene_h
#define planetaryScene_h

#include "../engine/core/scene.h"
#include "atmosphere.h"
#include "terrain.h"

class PlanetaryScene : public Scene
{
private:
	Atmosphere m_sky;
	Terrain m_terrain;
public:
	PlanetaryScene();
	~PlanetaryScene();

	void initAtmosphere();
	bool loadTerrain(int size, float range, std::shared_ptr<Material> material, std::shared_ptr<Texture> heightmap);

	void renderTerrain();
	void renderSky();
};

#endif