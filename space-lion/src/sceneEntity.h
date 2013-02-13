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
protected:
	const int id;

	glm::vec3 position;
	glm::quat orientation;
public:
	sceneEntity() : id(0), position(glm::vec3(0.0f)) {}
	sceneEntity(const int inId, glm::vec3 inPosition) : id(inId), position(inPosition){}
	~sceneEntity();

	void translate(glm::vec3);
	void rotate(const float angle,const glm::vec3 axis) {orientation = glm::rotate(orientation,angle,axis);}

	const int getId() {return id;}

	void setPosition(const glm::vec3 inPosition) {position = inPosition;}
	glm::vec3 getPosition() {return position;}

	void setOrientation(const glm::quat inOrientation) {orientation = inOrientation;}
	void setOrientation(const float angle, const glm::vec3 axis) {orientation = glm::rotate(glm::quat(0.0f,0.0f,0.0f,0.0f),angle,axis);}
	glm::quat getOrientation() {return orientation;}
};

#endif sceneEntity
