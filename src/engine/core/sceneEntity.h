#ifndef sceneEntity_h
#define sceneEntity_h

//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include "material.h"

class SceneEntity
{
protected:
	const int id;

	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scaling;
public:
	SceneEntity() : id(0), position(glm::vec3(0.0f)) {}
	SceneEntity(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation = glm::quat(), const glm::vec3& inScaling = glm::vec3(1.0))
		: id(inId), position(inPosition), orientation(inOrientation), scaling(inScaling) {}
	~SceneEntity();

	void translate(const glm::vec3& tvec) { position = position + tvec; }
	void rotate(const float angle,const glm::vec3& axis) {orientation = glm::normalize(glm::rotate(orientation,angle,axis));}

	const int getId() {return id;}

	void setPosition(const glm::vec3& inPosition) {position = inPosition;}
	glm::vec3 getPosition() {return position;}

	void setOrientation(const glm::quat& inOrientation) {orientation = inOrientation;}
	void setOrientation(const float angle, const glm::vec3& axis) {orientation = glm::rotate(glm::quat(),angle,axis);}
	const glm::quat& getOrientation() {return orientation;}

	void setScaling(const glm::vec3& inScaling) {scaling = inScaling;}
	glm::vec3 getScaling() {return scaling;}
};

#endif
