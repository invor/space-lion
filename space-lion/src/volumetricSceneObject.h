#ifndef volumetricSceneObject_h
#define volumetricSceneObject_h

#include "sceneEntity.h"
#include "texture3D.h"

class VolumetricSceneObject : public SceneEntity
{
private:
	bool isRendered;

	GLSLProgram* shaderPrgm;
	Texture3D* volume;

	/*
	/	Quick, propably not original, idea: use bounding geometry that roughly fits the shape of the volume for a limited empty space skipping.
	/	The bounding geometry would still be color coded, and the volume still be represented by a rectangular 3d texture.
	*/
	Mesh* boundingBoxGeom;
public:
	VolumetricSceneObject();
	~VolumetricSceneObject();

	VolumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, const glm::vec3& inScaling, Mesh* inGeom, Texture3D* inVolume, GLSLProgram* inPrgm)
		: SceneEntity(inId, inPosition, inOrientation, inScaling), boundingBoxGeom(inGeom), volume(inVolume), shaderPrgm(inPrgm) {}

	GLSLProgram* getShaderProgram() {return shaderPrgm;}
	Texture3D* getVolumeTexture() {return volume;}
	Mesh* getGeometry() {return boundingBoxGeom;}

	glm::mat4 computeModelMatrix() {return ( glm::translate(glm::mat4(1.0),position) * glm::scale(glm::mat4(1.0),scaling) * glm::mat4_cast(orientation) );}
};

#endif
