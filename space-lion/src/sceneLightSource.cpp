#include "sceneLightSource.h"


sceneLightSource::sceneLightSource()
{
}

sceneLightSource::~sceneLightSource()
{
}

sceneLightSource::sceneLightSource(int inId, glm::vec3 inPosition, glm::vec4 inLightColour) : sceneEntity(inId, inPosition)
{
	lightColour = inLightColour;
}