#ifndef sceneCamera_h
#define sceneCamera_h

//openGL Math Lib
#include "sceneEntity.h"

class SceneCamera : public SceneEntity
{
private:
	float aspectRatio;
	float fieldOfView;

public:
	SceneCamera();
	~SceneCamera();

	SceneCamera(const int, const glm::vec3&, const glm::quat&,float,float);

	void setAspectRation(const float inAspectRation) {aspectRatio = inAspectRation;}
	float getAspectRatio() {return aspectRatio;}
	void setFieldOfView(const float inFieldOfView) {fieldOfView = inFieldOfView;}
	float getFieldOfView() {return fieldOfView;}

	//	The initial camera orientation is defined as (0,0,-1)
	//	The current orientation is obtained by applying the orientation quaternion to the respective vectors 
	glm::vec3 computeFrontVector();
	glm::vec3 computeUpVector();
	glm::vec3 computeRightHandVector();

	glm::mat4 computeViewMatrix(){ return glm::lookAt(position,position+computeFrontVector(),computeUpVector()); }
	glm::mat4 computeProjectionMatrix(float nearClippingPlane, float farClippingPlane) { return glm::perspective(fieldOfView,aspectRatio,nearClippingPlane,farClippingPlane); }
};

#endif
