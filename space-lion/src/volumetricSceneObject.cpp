#include "volumetricSceneObject.h"


volumetricSceneObject::volumetricSceneObject()
{
}

volumetricSceneObject::~volumetricSceneObject()
{
}

volumetricSceneObject::volumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, vertexGeometry* inGeom, texture3D* inVolume) 
	: sceneEntity(inId, inPosition), boundingBoxGeom(inGeom), volume(inVolume)
{
}
