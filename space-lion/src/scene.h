#ifndef scene_h
#define scene_h

#include <list>

#include "sceneEntity.h"
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
	/	Some textures are not loaded from a file. Therefore an id will be generated for them by the scene.
	/	Texture id's below 10000 belong to textures loaded from a file. Id's above 10000 are given by the system.
	/	The most recently assigned id is saved in this variable.
	*/
	unsigned int lastTextureId;

	std::list<sceneEntity> scenegraph;
	std::list<vertexGeometry> vboList;
	std::list<material> materialList;
	std::list<texture> textureList;
	std::list<GLSLProgram> shaderProgramList;

	//	create a simple box object for debugging purposes 
	vertexGeometry* createVertexGeometry();
	//	create geometry from a local file
	vertexGeometry* createVertexGeometry(const char * const path);

	//	create default material for debugging purposes
	material* createMaterial();
	//	create material from local file
	material* createMaterial(const char * const path);
	//	for later use, when some kind of editor allows to change material properties at runtime
	bool reloadMaterial();

	//	create a shader program
	GLSLProgram* createShaderProgram(shaderType);

	/*
	/	create a texture from an array of float values
	/
	/	Exercise some caution when using this function. It will always create a new texture object,
	/	since for now there is no reasonable way to check if an identical texture already exsists.
	*/
	texture* createTexture(int dimX, int dimY, float* data);
	//	create a texture from file
	texture* createTexture(const char * const path);
	//	in case a texture file is changed during runtime
	bool reloadTexture();

	glm::mat4 computeMVP(const glm::vec3 position, const glm::vec4 orientation);

public:
	scene();
	~scene();

	//	create a scene entity with default geometry and default material
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation);
	//	create a scene entity with default geometry
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation, const char * const geometryPath);
	//	create a scene entity
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation, const char * const geometryPath, const char * const materialPath);

	void render();
};

#endif