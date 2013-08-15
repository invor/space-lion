#ifndef sceneLightSource_h
#define sceneLightSource_h

//openGL Math Lib
#include "sceneEntity.h"

class SceneLightSource : public SceneEntity
{
private:
	glm::vec4 lightColour;
public:
	SceneLightSource();
	~SceneLightSource();

	SceneLightSource(int, const glm::vec3&, const glm::vec4&);

	glm::vec4 getColour() {return lightColour;}
};

#endif
