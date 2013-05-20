#ifndef volumetricSceneObject_h
#define volumetricSceneObject_h

#include "sceneEntity.h"
#include "texture3D.h"

class volumetricSceneObject : public sceneEntity
{
private:
	bool isRendered;

	texture3D* volume;
	vertexGeometry* boundingBoxGeom;
public:
	volumetricSceneObject();
	~volumetricSceneObject();

	volumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, vertexGeometry* inGeom, texture3D* inVolume);

	vertexGeometry* getGeometry() {return boundingBoxGeom;}

	glm::mat4 computeModelMatrix() {return (glm::translate(glm::mat4(1.0),position))*(glm::mat4_cast(orientation));}
};

#endif
