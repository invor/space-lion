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

	sceneLightSource(int, const glm::vec3&, const glm::vec4&);

	glm::vec4 getColour() {return lightColour;}
};

#endif
