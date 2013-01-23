#ifndef sceneEntity_h
#define sceneEntity_h

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
	//orentation is saved as a quaternion, thus a vec4
	glm::vec4 orientation;

	bool isRendered;

	vertexBufferObject *geometry;
	material *matl;
public:
	sceneEntity(void);
	~sceneEntity(void);

	void setPosition(const glm::vec3 inPosition);
	glm::vec3 getPosition();
	void setOrientation(const glm::vec4 inOrientation);
	glm::vec4 getOrientation();
};

#endif sceneEntity
