#include "staticSceneObject.h"


staticSceneObject::staticSceneObject()
{
}

staticSceneObject::~staticSceneObject()
{
}

staticSceneObject::staticSceneObject(const int inId, vertexGeometry* inGeom, material* inMtl) : sceneEntity(inId)
{
	geometry = inGeom;
	mtl = inMtl;
}