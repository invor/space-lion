//#pragma once

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertexBufferObject.h"
#include "material.h"

class sceneEntity
{
private:
	glm::vec3 position;

	bool isRendered;

	vertexBufferObject *geometry;
	material *matl;
public:
	sceneEntity(void);
	~sceneEntity(void);
};

