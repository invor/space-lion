#ifndef scene_h
#define scene_h

#include <list>

#include "sceneEntity.h"
#include "staticSceneObject.h"
#include "sceneCamera.h"
#include "sceneLightSource.h"
#include "vertexGeometry.h"
#include "material.h"
#include "texture.h"
#include "GLSLProgram.h"

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"opengl32.lib")
#endif

class scene
{
private:
	/*	
	/	The following has become somewhat obsolete with the introdcution of filenames as identifier
	/
	/	Some textures are not loaded from a file. Therefore an id will be generated for them by the scene.
	/	Texture id's below 10000 belong to textures loaded from a file. Id's above 10000 are given by the system.
	/	The most recently assigned id is saved in this variable.
	*/
	unsigned int lastTextureId;

	std::list<sceneLightSource> lightSourceList;
	std::list<sceneCamera> cameraList;
	std::list<staticSceneObject> scenegraph;
	std::list<vertexGeometry> vboList;
	std::list<material> materialList;
	std::list<texture> textureList;
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
	bool createTexture(int dimX, int dimY, float* data, texture*& inOutTexPtr);
	//	create a texture from file, obtains a reference to the newly created texture via in-out parameter
	bool createTexture(const char * const path, texture*& inOutTexPtr);
	//	in case a texture file is changed during runtime
	bool reloadTexture();

	glm::mat4 computeModelMatrix(const glm::vec3 position, const glm::quat orientation);

public:
	scene();
	~scene();

	//	create a scene entity with default geometry and default material
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation);
	//	create a scene entity with default geometry
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, char * const geometryPath);
	//	create a scene entity
	bool createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const char * const geometryPath, const char * const materialPath);

	//	create a scene light source
	bool createSceneLight(const int id, const glm::vec3 position, glm::vec4 lightColour);
	//	create a scene camera
	bool createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientations, float aspect, float fov);

	void setActiveCamera(const int);

	//	create a
	void render();
};

#endif