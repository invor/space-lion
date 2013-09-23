/*
---------------------------------------------------------------------------------------------------
File: ftv_scene.h
Author: Michael Becher
Date of (presumingly) last edit: 29.05.2013

This C++ class is developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: Extension of the scene class, that adds ftv volume rendering.
---------------------------------------------------------------------------------------------------
*/

#ifndef ftv_scene_h
#define ftv_scene_h

#include <list>

#include "scene.h"
#include "ftv_volumetricSceneObject.h"

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"opengl32.lib")
#endif

class Ftv_Scene : public Scene
{
private:
	std::list<Ftv_volumetricSceneObject> ftv_volumetricSceneObjectList;

public:
	Ftv_Scene() {}
	~Ftv_Scene() {}

	/* create a volumetric scene entity for ftv testing */
	bool createFtvVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
											std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Texture3D> volPtr, std::shared_ptr<GLSLProgram> prgmPtr);

	/*
	/	Render volumetric objects of the scene using a mask to indicate faulty regions in the 3d texture.
	*/
	void ftvRenderVolumetricObjects();
};

#endif
