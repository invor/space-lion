#include "sceneLightSource.h"


SceneLightSource::SceneLightSource()
{
}

SceneLightSource::~SceneLightSource()
{
}

SceneLightSource::SceneLightSource(int inId, const glm::vec3& inPosition, const glm::vec4& inLightColour) : SceneEntity(inId, inPosition)
{
	lightColour = inLightColour;
}
