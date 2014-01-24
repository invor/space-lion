#ifndef terrain_h
#define terrain_h

/*	space-lion includes */
#include "../engine/core/mesh.h"
#include "../engine/core/material.h"

/**
* \class Terrain
*
* \brief Basic heightmap-based terrain
*
* A simple terrain implementation. Uses instanced quads a base geometry
* and heightmap-based vertex displacement to create a terrain mesh.
* Terrain/Heightmap have square size.
*
* \author Michael Becher
*
* \date 22nd January 2014
*/
class Terrain
{
private:
	/**
	* Size in meters. One quad is used per square meter.
	*/
	int m_size;
	/**
	* Maximum altitude of the terrain i.e. the height, that the highest possible value of the heightmap is mapped to.
	*/
	GLfloat m_range;

	Mesh m_quad;
	std::shared_ptr<Material> m_base_material;
	std::shared_ptr<Texture> m_heightmap;

public:
	Terrain();
	~Terrain();

	Terrain(int size, GLfloat range);

	/**
	* \brief Initialize resources for the terrain.
	* \note The base quad mesh is currently stored locally in the class
			while the resource manager keeps possesion of all other resources
	* \param material Pointer to the material for the terrain surface
	* \param heightmap Pointer to the texture file containing the heightmap
	* \return Returns true if resources could be created and/or validated, false otherwise
	*/
	bool init(std::shared_ptr<Material> material, std::shared_ptr<Texture> heightmap);

	/**
	* \brief Render terrain mesh.
	*/
	void render();

	/**
	* \brief Set new terrain size.
	* \param size New size, i.e. edge length (terrain is assumed to be square)
	*/
	void setSize(int size);
	/**
	* \brief Get current terrain size.
	* \return Returns edge length of terrain.
	*/
	int getSize();
	/**
	* \brief Set new maximum terrain height.
	* \param range New maximum terrain height
	*/
	void setRange(GLfloat range);
	/**
	* \brief Get current maximum terrain height.
	* \return Returns the maximum height of the terrain
	*/
	GLfloat getRange();


	std::shared_ptr<Material> getMaterial();
};

#endif