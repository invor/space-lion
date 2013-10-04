/*
---------------------------------------------------------------------------------------------------
File: ftv_scene.h
Author: Michael Becher
Date of (presumingly) last edit: 29.05.2013

This C++ class is developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: Extends the volumetricSceneObject class. Simply adds the handle for another
3d texture, which should contain a 3d mask for the modified volume rendering.
---------------------------------------------------------------------------------------------------
*/

#ifndef ftv_volumetricSceneObject_h
#define ftv_volumetricSceneObject_h

#include "../engine/core/volumetricSceneObject.h"
#include "../engine/core/texture3D.h"

class Ftv_volumetricSceneObject : public VolumetricSceneObject
{
private:
	Texture3D* ftvMaskVolume;
public:
	Ftv_volumetricSceneObject() {}
	~Ftv_volumetricSceneObject() {}

	Ftv_volumetricSceneObject(const int inId, const glm::vec3& inPosition, const glm::quat& inOrientation, const glm::vec3& inScaling,
		std::shared_ptr<Mesh> inGeom, std::shared_ptr<Texture3D> inVolume, Texture3D *inFtvMaskVolume, std::shared_ptr<GLSLProgram> inPrgm)
		: VolumetricSceneObject(inId,inPosition,inOrientation,inScaling,inGeom,inVolume,inPrgm), ftvMaskVolume(inFtvMaskVolume) {}

	Texture3D* getFtvMaskVolumeTexture() {return ftvMaskVolume;}
};

#endif
