#ifndef sceneLightSource_h
#define sceneLightSource_h

//openGL Math Lib
#include "sceneEntity.h"

class sceneLightSource : public sceneEntity
{
private:
	glm::vec4 lightColour;
public:
	sceneLightSource();
	~sceneLightSource();

	sceneLightSource(int,glm::vec3,glm::vec4);

	glm::vec4 getColour() {return lightColour;}
};

#endif sceneLightSource
