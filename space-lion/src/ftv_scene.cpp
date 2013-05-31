#include "ftv_scene.h"

bool ftv_scene::createFtvVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling, float* volumeData, GLenum internalFormat, GLenum format, const glm::ivec3 volumeRes)
{
	vertexGeometry* geomPtr;
	texture3D* volPtr;
	GLSLProgram* prgmPtr;

	if(!createVertexGeometry(geomPtr)) return false;
	if(!createTexture3D(volumeData,volumeRes,internalFormat,format,volPtr)) return false;
	if(!createShaderProgram(VOLUME_RAYCASTING,prgmPtr)) return false;

	volumetricObjectList.push_back(volumetricSceneObject(id,position,orientation,scaling,geomPtr,volPtr,prgmPtr));
	return true;
}

void ftv_scene::ftvRenderVolumetricObjects()
{
	/*	obtain transformation matrices */
	glm::mat4 modelViewMx;
	glm::mat4 modelViewProjectionMx;
	glm::mat4 textureMatrix;

	glm::mat4 modelMx;
	glm::mat4 viewMx(activeCamera->computeViewMatrix());
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,100.0f));

	GLSLProgram* currentPrgm(ftv_volumetricSceneObjectList.begin()->getShaderProgram());
	currentPrgm->use();

	for(std::list<ftv_volumetricSceneObject>::iterator i = ftv_volumetricSceneObjectList.begin(); i != ftv_volumetricSceneObjectList.end(); ++i)
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

		(i->getGeometry())->draw(GL_TRIANGLES,36,0);
		i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
}