#ifndef volumetricSceneObject_h
#define volumetricSceneObject_h

#include "sceneEntity.h"
#include "texture3D.h"

class volumetricSceneObject : public sceneEntity
{
private:
	bool isRendered;

	GLSLProgram* shaderPrgm;
	texture3D* volume;
	vertexGeometry* boundingBoxGeom;
public:
	volumetricSceneObject();
	~volumetricSceneObject();

	volumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, const glm::vec3& inScaling, vertexGeometry* inGeom, texture3D* inVolume, GLSLProgram* inPrgm)
		: sceneEntity(inId, inPosition, inOrientation, inScaling), boundingBoxGeom(inGeom), volume(inVolume), shaderPrgm(inPrgm) {}

	GLSLProgram* getShaderProgram() {return shaderPrgm;}
	texture3D* getVolumeTexture() {return volume;}
	vertexGeometry* getGeometry() {return boundingBoxGeom;}

	glm::mat4 computeModelMatrix() {return ( glm::translate(glm::mat4(1.0),position) * glm::scale(glm::mat4(1.0),scaling) * glm::mat4_cast(orientation) );}
};

#endif
