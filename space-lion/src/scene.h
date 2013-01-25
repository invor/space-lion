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

	texture* createTexture();
	GLSLProgram* createShaderProgram();

	glm::mat4 computeMVP(const glm::vec3 position, const glm::vec4 orientation);

public:
	scene(void);
	~scene(void);

	//	create a scene entity with default geometry and default material
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation);
	//	create a scene entity with default geometry
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation, const char * const geometryPath);
	//	create a scene entity
	bool createSceneEntity(const glm::vec3 position, const glm::vec4 orientation, const char * const geometryPath, const char * const materialPath);

	void render();
};

#endif