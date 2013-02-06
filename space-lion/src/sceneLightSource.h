#ifndef sceneLightSource_h
#define sceneLightSource_h

//openGL Math Lib
#include "sceneEntity.h"

class sceneLightSource : public sceneEntity
{
private:
	glm::bvec4 lightColour;
public:
	sceneLightSource();
	~sceneLightSource();
};

#endif sceneLightSource
