#include "staticSceneObject.h"


staticSceneObject::staticSceneObject()
{
}

staticSceneObject::~staticSceneObject()
{
}

staticSceneObject::staticSceneObject(const int inId, const glm::vec3& inPosition, vertexGeometry* inGeom, material* inMtl) : sceneEntity(inId, inPosition), geometry(inGeom), mtl(inMtl)
{
}
