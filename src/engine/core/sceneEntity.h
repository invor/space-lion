/**
* \class SceneEntity
*
* \brief Base class for scene entity
*
* This class contains the id and spatial information of a scene entity
* and offers basic functionality to manipulate those values, e.g. translate, rotate etc.
*
* \author Michael Becher
*
* \date 11th September 2013
*/

#ifndef sceneEntity_h
#define sceneEntity_h

/*	OpenGL Math Lib */
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SceneEntity
{
protected:
	const int id;

	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scaling;
public:
	SceneEntity() : id(0), position(glm::vec3(0.0f)) {}
	~SceneEntity() {}

	SceneEntity(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, const glm::vec3& inScaling)
		: id(inId), position(inPosition), orientation(inOrientation), scaling(inScaling) {}

	/**
	* \brief Change entity position via translation
	* \param tvec Translation vector
	*/
	void translate(const glm::vec3& tvec) { position = position + tvec; }

	/**
	* \brief Rotate entity by a given angle around a given vector
	* \param angle Rotation angle given in degree
	* \param axis Local rotation vector, e.g. vec3(1.0,0.0,0.0) is always the right hand vector, not the global x-axis
	*/
	void rotate(const float angle,const glm::vec3& axis) {orientation = glm::normalize(glm::rotate(orientation,angle,axis));}

	/**
	* \brief Get entity id
	* \return Returns entity id as an integer
	*/
	const int getId() {return id;}

	/**
	* \brief Set entity position
	* \param inPosition New position
	*/
	void setPosition(const glm::vec3& inPosition) {position = inPosition;}

	/**
	* \brief Get entity position
	* \return Returns position
	*/
	glm::vec3 getPosition() {return position;}

	void setOrientation(const glm::quat& inOrientation) {orientation = inOrientation;}
	void setOrientation(const float angle, const glm::vec3& axis) {orientation = glm::rotate(glm::quat(),angle,axis);}
	const glm::quat& getOrientation() {return orientation;}

	void setScaling(const glm::vec3& inScaling) {scaling = inScaling;}
	glm::vec3 getScaling() {return scaling;}
};

#endif
