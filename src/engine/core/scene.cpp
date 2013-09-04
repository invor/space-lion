#include "scene.h"


Scene::Scene()
{
}

Scene::~Scene()
{
}

bool Scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Material> mtlPtr)
{
	scenegraph.push_back(StaticSceneObject(id,position,orientation,geomPtr,mtlPtr));
	return true;
}

bool Scene::createVolumetricSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
											std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Texture3D> volPtr, std::shared_ptr<GLSLProgram> prgmPtr)
{
	volumetricObjectList.push_back(VolumetricSceneObject(id,position,orientation,scaling,geomPtr,volPtr,prgmPtr));
	return true;
}

bool Scene::createSceneLight(const int id, const glm::vec3 position, glm::vec3 colour)
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

SceneCamera* Scene::getActiveCamera()
{
	return activeCamera;
}

void Scene::testing()
{
	//	Ehhh, a few lines for testing
	//activeCamera->rotate(90,glm::vec3(0.0,0.0,1.0));
	activeCamera->rotate(45.0,glm::vec3(0.0,1.0,0.0));
	activeCamera->rotate(-22.5,glm::vec3(1.0,0.0,0.0));
	
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
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,5000.0f));

	std::shared_ptr<GLSLProgram> currentPrgm;
	std::shared_ptr<Material> currentMtl;

	for(std::list<StaticSceneObject>::iterator i = scenegraph.begin(); i != scenegraph.end(); ++i)
	{
		modelMx = i->computeModelMatrix();
		modelViewMx = viewMx * modelMx;
		normalMx = glm::transpose(glm::inverse(glm::mat3(modelViewMx)));
		modelViewProjectionMx = projectionMx * viewMx * modelMx;

		currentMtl = i->getMaterial();
		currentPrgm = currentMtl->getShaderProgram();
		

		currentPrgm->use();

		currentPrgm->setUniform("normal_matrix",normalMx);
		currentPrgm->setUniform("view_matrix", viewMx);
		currentPrgm->setUniform("model_view_matrix",modelViewMx);
		currentPrgm->setUniform("model_view_projection_matrix",modelViewProjectionMx);

		int light_counter = 0;
		std::string uniform_name;
		for(std::list<SceneLightSource>::iterator light_itr = lightSourceList.begin(); light_itr != lightSourceList.end(); ++light_itr)
		{
			uniform_name = "lights.position["+ std::to_string(light_counter);
			uniform_name.append("]");
			currentPrgm->setUniform(uniform_name.c_str(),light_itr->getPosition());
			
			uniform_name = "lights.intensity["+ std::to_string(light_counter);
			uniform_name.append("]");
			currentPrgm->setUniform(uniform_name.c_str(),light_itr->getColour());

		   
			//currentPrgm->setUniform("lights.position[0]",light_itr->getPosition());
			//currentPrgm->setUniform("lights.intensity[0]",light_itr->getColour());

			light_counter++;
			if(light_counter>=20) break;
		}
		currentPrgm->setUniform("num_lights",light_counter);

		currentMtl->use();

		(i->getGeometry())->draw();
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

	std::shared_ptr<GLSLProgram> currentPrgm(volumetricObjectList.begin()->getShaderProgram());
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

		(i->getGeometry())->draw();
		i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
}