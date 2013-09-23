#ifndef sceneCamera_h
#define sceneCamera_h

#include <iostream>

#include "sceneEntity.h"

class SceneCamera : public SceneEntity
{
private:
	float aspect_ratio;
	float field_of_view;

	glm::vec3 look_at;

public:
	SceneCamera() {}
	~SceneCamera() {}

	SceneCamera(int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, float inAspect, float inFov)
		: SceneEntity(inId, inPosition, inOrientation, glm::vec3(1.0f)), look_at(glm::vec3(0.0f)), aspect_ratio(inAspect), field_of_view(inFov) {}

	SceneCamera(int inId, const glm::vec3& inPosition, const glm::vec3& inLookAt, float inAspect, float inFov)
		: SceneEntity(inId, inPosition, glm::quat(), glm::vec3(1.0f)), look_at(inLookAt), aspect_ratio(inAspect), field_of_view(inFov)
	{
		/*	Compute initial orientation as given by position and look-at-point */
		glm::vec3 difference_vec = glm::normalize(inLookAt - inPosition);
		glm::vec3 projected_vec = glm::normalize(glm::vec3(difference_vec.x, 0.0f, difference_vec.z));
		float rot_angle_0 = acos(glm::dot(projected_vec, difference_vec));
		float rot_angle_1 = acos(glm::dot(projected_vec, glm::vec3(0.0f, 0.0f, -1.0f)));
		
		glm::quat orientation_0 = glm::rotate(glm::quat(), -(rot_angle_0 / 3.1415926535f)*180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat orientation_1 = glm::rotate(glm::quat(), (rot_angle_1 / 3.1415926535f)*180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		setOrientation(orientation_1*orientation_0);
		
	}

	void setAspectRation(const float inAspectRation) { aspect_ratio = inAspectRation; }
	float getAspectRatio() { return aspect_ratio; }
	void setFieldOfView(const float inFieldOfView) { field_of_view = inFieldOfView; }
	float getFieldOfView() { return field_of_view; }
	void setLookAt(const glm::vec3 inLookAt) { look_at = inLookAt; }
	glm::vec3 getLookAt() { return look_at; }

	/*	
	/	The initial camera orientation is defined as (0,0,-1).
	/	The current orientation is obtained by applying the orientation quaternion to the respective vectors.
	*/
	glm::vec3 computeFrontVector();
	glm::vec3 computeUpVector();
	glm::vec3 computeRightHandVector();

	glm::mat4 computeViewMatrix(){ return glm::lookAt(position,position+computeFrontVector(),computeUpVector()); }
	glm::mat4 computeProjectionMatrix(float nearClippingPlane, float farClippingPlane) { return glm::perspective(field_of_view, aspect_ratio, nearClippingPlane, farClippingPlane); }
};

#endif
