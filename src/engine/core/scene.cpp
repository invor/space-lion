#include "scene.h"


Scene::Scene()
{
}

Scene::~Scene()
{
}

bool Scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const glm::vec3 scaling,
										std::shared_ptr<Mesh> geomPtr, std::shared_ptr<Material> mtlPtr)
{
	std::shared_ptr<GLSLProgram> prgmPtr = mtlPtr->getShaderProgram();

	/*
	/	Push the new scene entity into the datastructure.
	/
	/	The used iterators will either point to a newly created list/map or
	/	an already existing one. In both cases the retrieved iterator can be used to insert
	/	a new map (or push the new entity into the list) or find an already existing one
	/	to continue on to the bottom of the datastructure.
	*/

	std::pair<ShaderMap::iterator, bool> itr_firstlevel =
		render_graph.insert(std::pair<std::shared_ptr<GLSLProgram>, MaterialMap>(prgmPtr,MaterialMap()));

	std::pair<MaterialMap::iterator, bool> itr_secondlevel =
		itr_firstlevel.first->second.insert(std::pair<std::shared_ptr<Material>, MeshMap>(mtlPtr, MeshMap()));
		
	std::pair<MeshMap::iterator, bool> itr_thirdlevel =
		itr_secondlevel.first->second.insert(std::pair<std::shared_ptr<Mesh>,std::list<StaticSceneObject>>(geomPtr,std::list<StaticSceneObject>()));

	itr_thirdlevel.first->second.push_back(StaticSceneObject(id,position,orientation,scaling,geomPtr,mtlPtr));

	//static_entity_list.push_back(StaticSceneObject(id,position,orientation,scaling,geomPtr,mtlPtr));
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

bool Scene::createSceneCamera(const int id, const glm::vec3 position, const glm::vec3 lookAt, float aspect, float fov)
{
	cameraList.push_back(SceneCamera(id, position, lookAt, aspect, fov));
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
void Scene::drawFroward()
{
	glm::mat4 modelMx;
	glm::mat4 viewMx;
	glm::mat4 modelViewMx;
	glm::mat4 projectionMx;
	
	/*	Iterate through all levels of the rendergraph */
	for (ShaderMap::iterator shader_itr = render_graph.begin(); shader_itr != render_graph.end(); ++shader_itr)
	{
		shader_itr->first->use();
		
		/*	Set "per program" uniforms */
		viewMx = activeCamera->computeViewMatrix();
		projectionMx = activeCamera->computeProjectionMatrix(1.0f,500000.0f);
		shader_itr->first->setUniform("view_matrix", viewMx);
		shader_itr->first->setUniform("projection_matrix", projectionMx);
	
		int light_counter = 0;
		shader_itr->first->setUniform("lights.position", lightSourceList.begin()->getPosition());
		shader_itr->first->setUniform("lights.intensity", lightSourceList.begin()->getColour());
		shader_itr->first->setUniform("num_lights", light_counter);
	
		for (MaterialMap::iterator material_itr = shader_itr->second.begin(); material_itr != shader_itr->second.end(); ++material_itr)
		{
			material_itr->first->use();
	
			for (MeshMap::iterator mesh_itr = material_itr->second.begin(); mesh_itr != material_itr->second.end(); ++mesh_itr)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;
				
				for (std::list<StaticSceneObject>::iterator entity_itr = mesh_itr->second.begin(); entity_itr != mesh_itr->second.end(); ++entity_itr)
				{
					/*	Set transformation matrix for each instance */
					modelMx = entity_itr->computeModelMatrix();
					modelViewMx = viewMx*modelMx;
					uniform_name = "model_view_matrix[" + std::to_string(instance_counter) + "]";
					shader_itr->first->setUniform(uniform_name.c_str(), modelViewMx);

					instance_counter++;
				}

				mesh_itr->first->draw(instance_counter);

				//for (std::list<StaticSceneObject>::iterator entity_itr = mesh_itr->second.begin(); entity_itr != mesh_itr->second.end(); ++entity_itr)
				//{
				//	modelMx = entity_itr->computeModelMatrix();
				//	modelViewMx = viewMx*modelMx;
				//	shader_itr->first->setUniform("model_view_matrix[0]", modelViewMx);
				//	mesh_itr->first->draw();
				//}
			}
		}
	}

	///*	obtain transformation matrices */
	//glm::mat4 modelMx;
	//glm::mat4 modelViewMx;
	//glm::mat4 viewMx(activeCamera->computeViewMatrix());
	//glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(1.0f,500000.0f));
	//glm::mat4 viewProjectionMx = projectionMx * viewMx;
	//
	//std::shared_ptr<GLSLProgram> currentPrgm;
	//std::shared_ptr<Material> currentMtl;
	//
	//for(std::list<StaticSceneObject>::iterator i = scenegraph.begin(); i != scenegraph.end(); ++i)
	//{
	//	modelMx = i->computeModelMatrix();
	//	modelViewMx = viewMx * modelMx;
	//
	//	currentMtl = i->getMaterial();
	//	currentPrgm = currentMtl->getShaderProgram();
	//	
	//	currentPrgm->use();
	//
	//	currentPrgm->setUniform("model_view_matrix[0]", modelViewMx);
	//	currentPrgm->setUniform("view_matrix", viewMx);
	//	currentPrgm->setUniform("projection_matrix", projectionMx);
	//
	//	int light_counter = 0;
	//	std::string uniform_name;
	//	//for(std::list<SceneLightSource>::iterator light_itr = lightSourceList.begin(); light_itr != lightSourceList.end(); ++light_itr)
	//	//{
	//	//	uniform_name = "lights.position["+ std::to_string(light_counter);
	//	//	uniform_name.append("]");
	//	//	currentPrgm->setUniform(uniform_name.c_str(),light_itr->getPosition());
	//	//	
	//	//	uniform_name = "lights.intensity["+ std::to_string(light_counter);
	//	//	uniform_name.append("]");
	//	//	currentPrgm->setUniform(uniform_name.c_str(),light_itr->getColour());
	//	//
	//	//   
	//	//	//currentPrgm->setUniform("lights.position[0]",light_itr->getPosition());
	//	//	//currentPrgm->setUniform("lights.intensity[0]",light_itr->getColour());
	//	//
	//	//	light_counter++;
	//	//	if(light_counter>=20) break;
	//	//}
	//
	//	currentPrgm->setUniform("lights.position", lightSourceList.begin()->getPosition());
	//	currentPrgm->setUniform("lights.intensity", lightSourceList.begin()->getColour());
	//
	//	currentPrgm->setUniform("num_lights",light_counter);
	//
	//	currentMtl->use();
	//
	//	(i->getGeometry())->draw();
	//	//i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	//}
}

void Scene::drawPicking(std::shared_ptr<GLSLProgram> prgm)
{
	prgm->use();

	glm::mat4 model_mx;
	glm::mat4 view_mx = activeCamera->computeViewMatrix();
	glm::mat4 projection_mx = activeCamera->computeProjectionMatrix(1.0f, 500000.0f);
	glm::mat4 model_view_projection_mx;

	/*	Iterate through all levels of the rendergraph */
	for (ShaderMap::iterator shader_itr = render_graph.begin(); shader_itr != render_graph.end(); ++shader_itr)
	{	
		for (MaterialMap::iterator material_itr = shader_itr->second.begin(); material_itr != shader_itr->second.end(); ++material_itr)
		{
			for (MeshMap::iterator mesh_itr = material_itr->second.begin(); mesh_itr != material_itr->second.end(); ++mesh_itr)
			{
				/*	Draw all entities instanced */
				int instance_counter = 0;
				std::string uniform_name;

				for (std::list<StaticSceneObject>::iterator entity_itr = mesh_itr->second.begin(); entity_itr != mesh_itr->second.end(); ++entity_itr)
				{
					/*	Set transformation matrix for each instance */
					model_mx = entity_itr->computeModelMatrix();
					model_view_projection_mx = projection_mx*view_mx*model_mx;
					uniform_name = "model_view_projection_matrix[" + std::to_string(instance_counter) + "]";
					prgm->setUniform(uniform_name.c_str(), model_view_projection_mx);

					uniform_name = "entity_id[" + std::to_string(instance_counter) + "]";
					prgm->setUniform(uniform_name.c_str(), entity_itr->getId());

					instance_counter++;
				}

				mesh_itr->first->draw(instance_counter);
			}
		}
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