#include "sceneEntity.h"


sceneEntity::sceneEntity() : id(0)
{
}

sceneEntity::~sceneEntity()
{
}

sceneEntity::sceneEntity(const int inId, vertexGeometry* inGeom, material* inMtl) : id(inId)
{
	geometry = inGeom;
	mtl = inMtl;
}