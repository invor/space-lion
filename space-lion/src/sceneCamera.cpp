#include "sceneCamera.h"


sceneCamera::sceneCamera()
{
}

sceneCamera::~sceneCamera()
{
}

sceneCamera::sceneCamera(int inId, glm::vec3 inPosition, glm::quat inOrientation, float inAspect, float inFov) : sceneEntity(inId,inPosition)
{
	aspectRatio = inAspect;
	fieldOfView = inFov;
}

void sceneCamera::rotateCamera(const glm::quat rotation)
{
	orientation = orientation * rotation;
}

glm::vec3 sceneCamera::computeFrontVector()
{
	//	Use a pure quaternion containing the initial front vector to compute a pure quaternion
	//	containing the front vector according the the cameras orientation
	glm::quat qFront = (orientation * glm::quat(0.0,0.0,0.0,-1.0) * glm::conjugate(orientation));
	return glm::normalize(glm::vec3(qFront.x,qFront.y,qFront.z));
}

glm::vec3 sceneCamera::computeUpVector()
{
	//	Use a pure quaternion containing the initial front vector to compute a pure quaternion
	//	containing the front vector according the the cameras orientation
	glm::quat qFront = (orientation * glm::quat(0.0,0.0,1.0,0.0) * glm::conjugate(orientation));
	return glm::normalize(glm::vec3(qFront.x,qFront.y,qFront.z));
}

glm::vec3 sceneCamera::computeRightHandVector()
{
	//	Use a pure quaternion containing the initial front vector to compute a pure quaternion
	//	containing the front vector according the the cameras orientation
	glm::quat qFront = (orientation * glm::quat(0.0,1.0,0.0,0.0) * glm::conjugate(orientation));
	return glm::normalize(glm::vec3(qFront.x,qFront.y,qFront.z));
}

glm::mat4 sceneCamera::computeViewMatrix()
{
	return glm::lookAt(position,position+computeFrontVector(),computeUpVector());
}

glm::mat4 sceneCamera::computeProjectionMatrix(float nearClippingPlane, float farClippingPlane)
{
	return glm::perspective(fieldOfView,aspectRatio,nearClippingPlane,farClippingPlane);
}