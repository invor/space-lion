#ifndef sceneCamera_h
#define sceneCamera_h

//openGL Math Lib
#include "sceneEntity.h"

class sceneCamera : public sceneEntity
{
private:
	glm::quat orientation;
	float aspectRation;
	float fieldOfView;

public:
	sceneCamera();
	~sceneCamera();

	void setOrientation(const glm::quat inOrientation) {orientation = inOrientation;}
	glm::quat getOrientation() {return orientation;}
};

#endif sceneCamera
