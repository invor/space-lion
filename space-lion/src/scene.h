//#pragma once
#include <list>

#include "sceneEntity.h"
#include "vertexBufferObject.h"
#include "material.h"
#include "texture.h"
#include "GLSLProgram.h"

class scene
{
private:
	std::list<sceneEntity> scenegraph;
	std::list<vertexBufferObject> vboList;
	std::list<material> materialList;
	std::list<texture> textureList;
	std::list<GLSLProgram> shaderProgramList;

	bool createVertexBufferObject();
	bool createMaterial();
	bool createTexture();
	bool createShaderProgram();
public:
	scene(void);
	~scene(void);

	bool createSceneEntity();
	void render();
};

