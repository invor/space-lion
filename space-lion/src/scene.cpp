#include "scene.h"


Scene::Scene()
{
}

Scene::~Scene()
{
}

bool Scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, Mesh* geomPtr, Material* mtlPtr)
{
	scenegraph.push_back(StaticSceneObject(id,position,geomPtr,mtlPtr));
	return true;
}

bool Scene::createVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling, Mesh* geomPtr, Texture3D* volPtr, GLSLProgram* prgmPtr)
{
	volumetricObjectList.push_back(VolumetricSceneObject(id,position,orientation,scaling,geomPtr,volPtr,prgmPtr));
	return true;
}

bool Scene::createSceneLight(const int id, const glm::vec3 position, glm::vec4 colour)
{
	lightSourceList.push_back(SceneLightSource(id, position, colour));
	return true;
}

bool Scene::createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientation, float aspect, float fov)
{
	cameraList.push_back(SceneCamera(id, position, orientation, aspect, fov));
	return true;
}

void Scene::setActiveCamera(const int inId)
{
	//	check camera list
	for(std::list<SceneCamera>::iterator i = cameraList.begin(); i != cameraList.end(); ++i)
	{
		if((i->getId())==inId)
		{
			activeCamera = &(*i);
		}
	}
}

void Scene::testing()
{
	//	Ehhh, a few lines for testing
	//activeCamera->rotate(12.5,glm::vec3(0.0,1.0,0.0));
	//activeCamera->rotate(-12.5,glm::vec3(1.0,0.0,0.0));
	
	//std::cout<<textureList.size();
	//std::cout<<vboList.size()<<"\n";
	//std::cout<<Scenegraph.size();

	//activeCamera->rotate(45.0,glm::vec3(0.0,1.0,0.0));
	//activeCamera->rotate(-45.0,glm::vec3(1.0,0.0,0.0));
	//glm::quat tVec = activeCamera->getOrientation();
	//std::cout<<"Orientation: "<<tVec.x<<" "<<tVec.y<<" "<<tVec.z<<" "<<tVec.w<<"\n";
	//
	//glm::vec3 fVec = activeCamera->computeFrontVector();
	//std::cout<<"Front: "<<fVec.x<<" "<<fVec.y<<" "<<fVec.z<<"\n";
	//glm::vec3 uVec = activeCamera->computeUpVector();
	//std::cout<<"Up: "<<uVec.x<<" "<<uVec.y<<" "<<uVec.z<<"\n";
	//glm::vec3 rVec = activeCamera->computeRightHandVector();
	//std::cout<<"Righthand: "<<rVec.x<<" "<<rVec.y<<" "<<rVec.z<<"\n";
	//
	//glm::vec3 lookat = ((activeCamera->getPosition())+(activeCamera->computeFrontVector()));
	//std::cout<<"Loookat: "<<lookat.x<<" "<<lookat.y<<" "<<lookat.z<<"\n
}

/*
/	Temporary render method
*/
void Scene::render()
{
	/*	obtain transformation matrices */
	glm::mat4 modelViewMx;
	glm::mat4 modelViewProjectionMx;
	glm::mat3 normalMx;

	glm::mat4 modelMx;
	glm::mat4 viewMx(activeCamera->computeViewMatrix());
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,500.0f));

	////
	//	access each entity of the Scene and draw it
	////
	//	if(materialList.size() > 0)
	//	{
	//	GLSLProgram* currentPrgm(&(*shaderProgramList.begin()));
	//	material* currentMtl(&(*materialList.begin()));
	//	currentPrgm->use();
	//	
	//	glEnable(GL_TEXTURE_2D);
	//	glActiveTexture(GL_TEXTURE0);
	//	currentPrgm->setUniform("diffuseMap",0);
	//	currentMtl->getDiffuseMap()->bindTexture();
	//	glActiveTexture(GL_TEXTURE1);
	//	currentPrgm->setUniform("specularMap",1);
	//	currentMtl->getSpecularMap()->bindTexture();
	//	glActiveTexture(GL_TEXTURE2);
	//	currentPrgm->setUniform("normalMap",2);
	//	currentMtl->getNormalMap()->bindTexture();

	GLSLProgram* currentPrgm;
	Material* currentMtl;

	for(std::list<StaticSceneObject>::iterator i = scenegraph.begin(); i != scenegraph.end(); ++i)
	{
		modelMx = i->computeModelMatrix();
		modelViewMx = viewMx * modelMx;
		normalMx = glm::transpose(glm::inverse(glm::mat3(modelViewMx)));
		modelViewProjectionMx = projectionMx * viewMx * modelMx;

		currentPrgm = (i->getMaterial())->getShaderProgram();
		currentMtl = i->getMaterial();

		currentPrgm->use();

		currentPrgm->setUniform("normalMatrix",normalMx);
		currentPrgm->setUniform("viewMatrix", viewMx);
		currentPrgm->setUniform("modelViewMatrix",modelViewMx);
		currentPrgm->setUniform("modelViewProjectionMatrix",modelViewProjectionMx);
		currentPrgm->setUniform("lightPosition",(lightSourceList.begin())->getPosition());
		currentPrgm->setUniform("lightColour",(lightSourceList.begin())->getColour());

		currentMtl->use();

		(i->getGeometry())->draw(GL_TRIANGLES,2000,0);
		//i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
}

void Scene::renderVolumetricObjects()
{
	/*	obtain transformation matrices */
	glm::mat4 modelViewMx;
	glm::mat4 modelViewProjectionMx;
	glm::mat4 textureMatrix;

	glm::mat4 modelMx;
	glm::mat4 viewMx(activeCamera->computeViewMatrix());
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,100.0f));

	GLSLProgram* currentPrgm(volumetricObjectList.begin()->getShaderProgram());
	currentPrgm->use();

	for(std::list<VolumetricSceneObject>::iterator i = volumetricObjectList.begin(); i != volumetricObjectList.end(); ++i)
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

		currentPrgm->setUniform("modelViewProjectionMatrix",modelViewProjectionMx);
		currentPrgm->setUniform("modelMatrix",modelMx);
		currentPrgm->setUniform("textureMatrix",textureMatrix);
		currentPrgm->setUniform("cameraPosition",activeCamera->getPosition());

		(i->getGeometry())->draw(GL_TRIANGLES,36,0);
		i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
}