#ifndef sceneEntity_h
#define sceneEntity_h

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertexGeometry.h"
#include "material.h"

class sceneEntity
{
private:
	const int id;

	glm::vec3 position;
public:
	sceneEntity();
	~sceneEntity();

	sceneEntity(const int);

	const int getId() {return id;}
	void setPosition(const glm::vec3 inPosition) {position = inPosition;}
	glm::vec3 getPosition() {return position;}
};

#endif sceneEntity
