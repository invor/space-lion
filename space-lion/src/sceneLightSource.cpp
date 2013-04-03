#include "sceneLightSource.h"


sceneLightSource::sceneLightSource()
{
}

sceneLightSource::~sceneLightSource()
{
}

sceneLightSource::sceneLightSource(int inId, const glm::vec3& inPosition, const glm::vec4& inLightColour) : sceneEntity(inId, inPosition)
{
	lightColour = inLightColour;
}
