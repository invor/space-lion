#ifndef sceneCamera_h
#define sceneCamera_h

//openGL Math Lib
#include "sceneEntity.h"

class sceneCamera : public sceneEntity
{
private:
	float aspectRatio;
	float fieldOfView;

public:
	sceneCamera();
	~sceneCamera();

	sceneCamera(const int,glm::vec3,glm::quat,float,float);

	void setAspectRation(const float inAspectRation) {aspectRatio = inAspectRation;}
	float getAspectRatio() {return aspectRatio;}
	void setFieldOfView(const float inFieldOfView) {fieldOfView = inFieldOfView;}
	float getFieldOfView() {return fieldOfView;}

	//	The initial camera orientation is defined as (0,0,-1)
	//	The current orientation is obtained by applying the orientation quaternion to the respective vectors 
	glm::vec3 computeFrontVector();
	glm::vec3 computeUpVector();
	glm::vec3 computeRightHandVector();

	glm::mat4 computeViewMatrix();
	glm::mat4 computeProjectionMatrix(float nearClippingPlane, float farClippingPlane);
};

#endif sceneCamera