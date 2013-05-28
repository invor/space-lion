#ifndef scene_h
#define scene_h

#include <list>

#include "sceneEntity.h"
#include "staticSceneObject.h"
#include "sceneCamera.h"
#include "sceneLightSource.h"
#include "volumetricSceneObject.h"
#include "ftv_volumetricSceneObject.h"
#include "vertexGeometry.h"
#include "material.h"
#include "texture2D.h"
#include "texture3D.h"
#include "GLSLProgram.h"
#include "resourceLoader.h"

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"opengl32.lib")
#endif

class scene
{
protected:
	/*	
	/	The following has become somewhat obsolete with the introdcution of filenames as identifier
	/
	/	Some textures are not loaded from a file. Therefore an id will be generated for them by the scene.
	/	Texture id's below 10000 belong to textures loaded from a file. Id's above 10000 are given by the system.
	/	The most recently assigned id is saved in this variable.
	*/
	unsigned int lastTextureId;
	resourceLoader resouceParser;

	/*
	/	The following lists contain all entities (objects if you will) that are part of the scene.
	/	The currently used std::list datastructures are to be replaced with a more sophisticated concept in the near future.
	*/
	std::list<sceneLightSource> lightSourceList;
	std::list<sceneCamera> cameraList;
	std::list<staticSceneObject> scenegraph;
	std::list<volumetricSceneObject> volumetricObjectList;

	/*
	/	The following lists contain all resources that are in use by an entity of this scene.
	/	There is only a single "instance" of any (uniquely identifiable) resouce kept in these lists.
	/	Different entities making use of the same resource, will both be refering to the single instance kept
	/	within these lists.
	*/
	std::list<vertexGeometry> vboList;
	std::list<material> materialList;
	/* This is kinda temporary. In the long run it might be nice to have all different texture types in a single list. */
	std::list<texture2D> textureList;
	/* Another temporary thing: volume masks used for ftv are stored in the volume list alongside the normal volume datasets */
	std::list<texture3D> volumeList;
	std::list<GLSLProgram> shaderProgramList;

	sceneCamera* activeCamera;


	//	debug method, create a triangle
	bool createTriangle(vertexGeometry*& inOutGeomPtr);
	//	create a simple box object for debugging purposes, obtains a reference to the newly created vertex geometry via in-out parameter
	bool createVertexGeometry(vertexGeometry*& inOutGeomPtr);
	//	create geometry from a local file
	bool createVertexGeometry(const char * const path, vertexGeometry*& inOutGeomPtr);

	//	create default material for debugging purposes, obtains a reference to the newly created material via in-out parameter
	bool createMaterial(material*& inOutMtlPtr);
	//	create material from local file, obtains a reference to the newly created material via in-out parameter
	bool createMaterial(const char * const path, material*& inOutMtlPtr);
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool reloadMaterial();

	//	create a shader program, obtains a reference to the newly created shader program via in-out parameter
	bool createShaderProgram(shaderType, GLSLProgram*& inOutPrgPtr);
	
	/*
	/	create a texture from an array of float values, obtains a reference to the newly created texture via in-out parameter
	/
	/	Exercise some caution when using this function. It will always create a new texture object,
	/	since for now there is no reasonable way to check if an identical texture already exsists.
	*/
	bool createTexture2D(int dimX, int dimY, float* data, texture*& inOutTexPtr);
	//	create a texture from file, obtains a reference to the newly created texture via in-out parameter
	bool createTexture2D(const std::string path, texture*& inOutTexPtr);
	//	in case a texture file is changed during runtime
	bool reloadTexture();

	/* create a 3D texture for volume rendering */
	bool createTexture3D(const std::string path, glm::ivec3 textureRes, texture3D*& inOutTexPtr);
	bool createTexture3D(float* volumeData, glm::ivec3 textureRes, GLenum internalFormat, GLenum format, texture3D*& inOutTexPtr);

public:
	scene();
	~scene();

	//	create a scene entity with default geometry and default material
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation);
	//	create a scene entity with default material or default geometry, based on the value of flag
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const char * const path, const bool flag);
	//	create a scene entity
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const char * const geometryPath, const char * const materialPath);

	/* create a volumetric scene entity */
	bool createVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling, const std::string path, const glm::ivec3 volumeRes);
	
	//	create a scene light source
	bool createSceneLight(const int id, const glm::vec3 position, glm::vec4 lightColour);
	//	create a scene camera
	bool createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientations, float aspect, float fov);

	void setActiveCamera(const int);

	void testing();

	/* render the scene */
	void render();

	/*
	/	Render the volumetric objects of the scene.
	/	This is usually be done in a sperate render pass to allow depth correct blending.
	*/
	void renderVolumetricObjects();
};

#endif
