/*
---------------------------------------------------------------------------------------------------
File: ftv_scene.h
Author: Michael Becher
Date of (presumingly) last edit: 29.05.2013

This C++ class is developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Describtion: Extends the volumetricSceneObject class. Simply adds the handle for another
3d texture, which should contain a 3d mask for the modified volume rendering.
---------------------------------------------------------------------------------------------------
*/

#ifndef ftv_volumetricSceneObject_h
#define ftv_volumetricSceneObject_h

#include "volumetricSceneObject.h"
#include "texture3D.h"

class ftv_volumetricSceneObject : public volumetricSceneObject
{
private:
	texture3D* ftvMaskVolume;
public:
	ftv_volumetricSceneObject() {}
	~ftv_volumetricSceneObject() {}

	ftv_volumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, const glm::vec3& inScaling,
		vertexGeometry* inGeom, texture3D* inVolume, texture3D *inFtvMaskVolume, GLSLProgram* inPrgm)
		: volumetricSceneObject(inId,inPosition,inOrientation,inScaling,inGeom,inVolume,inPrgm), ftvMaskVolume(inFtvMaskVolume) {}

	texture3D* getFtvMaskVolumeTexture() {return ftvMaskVolume;}
};

#endif
