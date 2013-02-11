#include "staticSceneObject.h"


staticSceneObject::staticSceneObject()
{
}

staticSceneObject::~staticSceneObject()
{
}

staticSceneObject::staticSceneObject(const int inId, glm::vec3 inPosition, vertexGeometry* inGeom, material* inMtl) : sceneEntity(inId, inPosition)
{
	geometry = inGeom;
	mtl = inMtl;
}

void staticSceneObject::rotate(const glm::quat rotation)
{
	orientation = orientation * rotation;
}

glm::mat4 staticSceneObject::computeModelMatrix()
{
	glm::mat4 mx = glm::rotate(glm::mat4(1.0),orientation.w,glm::vec3(orientation.x,orientation.y,orientation.z));
	mx = glm::translate(mx, position);
	return mx;
}