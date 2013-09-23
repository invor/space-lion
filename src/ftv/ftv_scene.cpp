#include "ftv_scene.h"

bool Ftv_Scene::createFtvVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
												std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Texture3D> volPtr, std::shared_ptr<GLSLProgram> prgmPtr)
{
	volumetricObjectList.push_back(VolumetricSceneObject(id,position,orientation,scaling,geomPtr,volPtr,prgmPtr));
	return true;
}

void Ftv_Scene::ftvRenderVolumetricObjects()
{
	/*	obtain transformation matrices */
	glm::mat4 modelViewMx;
	glm::mat4 modelViewProjectionMx;
	glm::mat4 textureMatrix;

	glm::mat4 modelMx;
	glm::mat4 viewMx(activeCamera->computeViewMatrix());
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,100.0f));

	std::shared_ptr<GLSLProgram> currentPrgm(ftv_volumetricSceneObjectList.begin()->getShaderProgram());
	currentPrgm->use();

	for(std::list<Ftv_volumetricSceneObject>::iterator i = ftv_volumetricSceneObjectList.begin(); i != ftv_volumetricSceneObjectList.end(); ++i)
	{
		modelMx = i->computeModelMatrix();
		modelViewMx = viewMx * modelMx;

		/*	construct the texture matrix */
		textureMatrix = glm::scale(glm::mat4(1.0),glm::vec3(1.0,1.0,-1.0));
		textureMatrix = glm::translate(textureMatrix,glm::vec3(0.5,0.5,-0.5));
		textureMatrix = textureMatrix * glm::inverse(modelMx);

		modelViewProjectionMx = projectionMx * viewMx * modelMx;

		glEnable(GL_TEXTURE_3D);
		glActiveTexture(GL_TEXTURE0);
		currentPrgm->setUniform("volumeTexture",0);
		i->getVolumeTexture()->bindTexture();

		glActiveTexture(GL_TEXTURE1);
		currentPrgm->setUniform("mask",1);
		i->getFtvMaskVolumeTexture()->bindTexture();

		currentPrgm->setUniform("modelViewProjectionMatrix",modelViewProjectionMx);
		currentPrgm->setUniform("modelMatrix",modelMx);
		currentPrgm->setUniform("textureMatrix",textureMatrix);
		currentPrgm->setUniform("cameraPosition",activeCamera->getPosition());

		(i->getGeometry())->draw();
		i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
}