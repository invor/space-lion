#ifndef scene_h
#define scene_h

#include <list>
#include <map>

#include "sceneEntity.h"
#include "staticSceneObject.h"
#include "sceneCamera.h"
#include "sceneLightSource.h"
#include "volumetricSceneObject.h"

class Scene
{
protected:
	/*
	/	The following lists contain all entities (objects if you will) that are part of the scene.
	/	The currently used std::list datastructures are to be replaced with a more sophisticated concept in the near future.
	*/
	std::list<SceneLightSource> lightSourceList;
	std::list<SceneCamera> cameraList;
	std::list<StaticSceneObject> static_entity_list;
	std::list<VolumetricSceneObject> volumetricObjectList;

	/*	Static scene entities (objects) sorted for optimzed drawing in a tree-like structure*/
	typedef std::map< std::shared_ptr<Mesh>, std::list<StaticSceneObject> > MeshMap;
	typedef std::map< std::shared_ptr<Material>, MeshMap > MaterialMap;
	typedef std::map< std::shared_ptr<GLSLProgram>, MaterialMap > ShaderMap;
	ShaderMap render_graph;

	SceneCamera* activeCamera;

public:
	Scene();
	~Scene();

	/* */
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
									std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Material> mtlPtr);
	
	/* create a volumetric scene entity */
	bool createVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
										std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Texture3D> volPtr, std::shared_ptr<GLSLProgram> prgmPtr);
	
	//	create a scene light source
	bool createSceneLight(const int id, const glm::vec3 position, glm::vec3 lightColour);
	//	create a scene camera
	bool createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientations, float aspect, float fov);
	//	create a scene camera
	bool createSceneCamera(const int id, const glm::vec3 position, const glm::vec3 lookAt, float aspect, float fov);

	void setActiveCamera(const int);
	SceneCamera* getActiveCamera();

	void testing();

	/*	draw the scene for a forward render pass */
	void drawFroward();
	
	/*	 draw the scene for a picking pass */
	void drawPicking(std::shared_ptr<GLSLProgram> prgm);

	/*
	/	Render the volumetric objects of the scene.
	/	This is usually be done in a sperate render pass to allow depth correct blending.
	*/
	void renderVolumetricObjects();
};

#endif
