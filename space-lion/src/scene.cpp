#include "scene.h"


scene::scene()
{
	lastTextureId = 10000;
}

scene::~scene()
{
}


bool scene::createTriangle(vertexGeometry*& inOutGeomPtr)
{
	vertex_pn *vertexArray = new vertex_pn[3];
	GLubyte *indexArray = new GLubyte[3];

	vertexArray[0]=vertex_pn(-0.5f,0.0f,0.0f,1.0f,0.0f,0.0f);
	vertexArray[1]=vertex_pn(0.5f,0.0f,0.0f,0.0f,1.0f,0.0f);
	vertexArray[2]=vertex_pn(0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);

	indexArray[0]=0;indexArray[1]=1;indexArray[2]=2;

	vboList.push_back(vertexGeometry("0"));
	std::list<vertexGeometry>::iterator lastElement = --(vboList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(vertex_pn)*3,sizeof(GLubyte)*3))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pn),0);
	lastElement->setVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pn),(GLvoid*) sizeof(vertex_p));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool scene::createVertexGeometry(vertexGeometry*& inOutGeomPtr)
{
	//	check list of vertexBufferObjects for default box object(filename="0")
	for(std::list<vertexGeometry>::iterator i = vboList.begin(); i != vboList.end(); ++i)
	{
		//	The "1024" is ugly but this should be changed later anyway.
		if(strncmp((i->getFilename()),"0",1024)){
			inOutGeomPtr = &(*i);
			return true;
		}
	}

	//	if default box not already in list, continue here
	vertex_pntcu *vertexArray = new vertex_pntcu[24];
	GLubyte *indexArray = new GLubyte[36];

	//	front face
	vertexArray[0]=vertex_pntcu(-0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,0,0.0,0.0);vertexArray[1]=vertex_pntcu(-0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[2]=vertex_pntcu(0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[3]=vertex_pntcu(0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	right face
	vertexArray[4]=vertex_pntcu(0.5,-0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,0.0,0.0);vertexArray[5]=vertex_pntcu(0.5,0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,0.0,1.0);
	vertexArray[6]=vertex_pntcu(0.5,0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,1.0,1.0);vertexArray[7]=vertex_pntcu(0.5,-0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,1.0,0.0);
	//	left face
	vertexArray[8]=vertex_pntcu(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,0.0,0.0);vertexArray[9]=vertex_pntcu(-0.5,0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,0.0,1.0);
	vertexArray[10]=vertex_pntcu(-0.5,0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,1.0,1.0);vertexArray[11]=vertex_pntcu(-0.5,-0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,1.0,0.0);
	//	back face
	vertexArray[12]=vertex_pntcu(0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[13]=vertex_pntcu(0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[14]=vertex_pntcu(-0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[15]=vertex_pntcu(-0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	bottom face
	vertexArray[16]=vertex_pntcu(-0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[17]=vertex_pntcu(-0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[18]=vertex_pntcu(0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[19]=vertex_pntcu(0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	top face
	vertexArray[20]=vertex_pntcu(-0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[21]=vertex_pntcu(-0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[22]=vertex_pntcu(0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[23]=vertex_pntcu(0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);

	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;
	indexArray[6]=4;indexArray[7]=6;indexArray[8]=5;
	indexArray[9]=6;indexArray[10]=4;indexArray[11]=7;
	indexArray[12]=8;indexArray[13]=10;indexArray[14]=9;
	indexArray[15]=10;indexArray[16]=8;indexArray[17]=11;
	indexArray[18]=12;indexArray[19]=14;indexArray[20]=13;
	indexArray[21]=14;indexArray[22]=12;indexArray[23]=15;
	indexArray[24]=16;indexArray[25]=17;indexArray[26]=18;
	indexArray[27]=18;indexArray[28]=19;indexArray[29]=16;
	indexArray[30]=20;indexArray[31]=22;indexArray[32]=21;
	indexArray[33]=22;indexArray[34]=20;indexArray[35]=23;

	vboList.push_back(vertexGeometry("0"));
	std::list<vertexGeometry>::iterator lastElement = --(vboList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray,sizeof(vertex_pntcu)*24,sizeof(GLubyte)*36))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),0);
	lastElement->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_p));
	lastElement->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pn));
	lastElement->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pnt));
	lastElement->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(vertex_pntcu),(GLvoid*) sizeof(vertex_pntc));

	inOutGeomPtr = &(*lastElement);
	return true;
}

bool scene::createVertexGeometry(const char * const path, vertexGeometry*& inOutGeomPtr)
{
	return false;
}

bool scene::createMaterial(material*& inOutMtlPtr)
{
	//	check list of materials for default material(id=0)
	for(std::list<material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==0)
		{
			inOutMtlPtr = &*i;
			return true;
		}
	}

	//	if default material is not already in list, create it and add it to list

	float* diffuseData = new float[4];
	float* specularData = new float[4];
	float* normalData = new float[4];
	//white diffuse texture
	diffuseData[0]=1.0f; diffuseData[1]=1.0f; diffuseData[2]=1.0f; diffuseData[3]=1.0f;
	//dark grey specular texture
	specularData[0]=0.3f; specularData[1]=0.3f; specularData[2]=0.3f; specularData[3]=1.0f;
	//normal pointing upwards
	normalData[0]=0.0f; normalData[1]=0.0f; normalData[2]=1.0f; normalData[3]=0.0f;
	
	GLSLProgram* prgPtr;
	texture* texPtr1;
	texture* texPtr2;
	texture* texPtr3;
	if(!createShaderProgram(FLAT,prgPtr)) return false;
	if(!createTexture(1,1,diffuseData,texPtr1)) return false;
	if(!createTexture(1,1,specularData,texPtr2)) return false;
	if(!createTexture(1,1,normalData,texPtr3)) return false;
	materialList.push_back(material(0,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool scene::createMaterial(const char * const path, material*& inOutMtlPtr)
{
	materialInfo inOutMtlInfo;
	if(!resouceParser.parseMaterial(path,inOutMtlInfo))return false;

	//	check list of materials for default material(id=0)
	for(std::list<material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==inOutMtlInfo.id)
		{
			inOutMtlPtr = &*i;
			return true;
		}
	}

	GLSLProgram* prgPtr;
	texture* texPtr1;
	texture* texPtr2;
	texture* texPtr3;
	if(!createShaderProgram(PHONG,prgPtr)) return false;
	if(!createTexture(inOutMtlInfo.diff_path,texPtr1)) return false;
	if(!createTexture(inOutMtlInfo.spec_path,texPtr2)) return false;
	if(!createTexture(inOutMtlInfo.normal_path,texPtr3)) return false;
	materialList.push_back(material(inOutMtlInfo.id,prgPtr,texPtr1,texPtr2,texPtr3));

	std::list<material>::iterator lastElement = --(materialList.end());
	inOutMtlPtr = &(*lastElement);
	return true;
}

bool scene::createShaderProgram(shaderType type, GLSLProgram*& inOutPrgPtr)
{
	//	check list of shader programs for the shader type
	for(std::list<GLSLProgram>::iterator i = shaderProgramList.begin(); i != shaderProgramList.end(); ++i)
	{
		if((i->getType())==type){
			inOutPrgPtr = &*i;
			return true;
		}
	}

	GLSLProgram shaderPrg;
	if(!shaderPrg.initShaders(type)) return false;
	shaderProgramList.push_back(shaderPrg);
	std::list<GLSLProgram>::iterator lastElement = --(shaderProgramList.end());
	inOutPrgPtr = &(*lastElement);
	return true;
}

bool scene::createTexture(int dimX, int dimY, float* data, texture*& inOutTexPtr)
{
	++lastTextureId;
	//	somewhat messy
	const char* tstr = "0";
	//_itoa(lastTextureId,tstr,10);
	textureList.push_back(texture2D());
	std::list<texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadArrayF(dimX, dimY, data))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool scene::createTexture(const std::string path, texture*& inOutTexPtr)
{
	for(std::list<texture2D>::iterator i = textureList.begin(); i != textureList.end(); ++i)
	{
		if((i->getFilename())==path)
		{
			inOutTexPtr = &*i;
			return true;
		}
	}

	textureList.push_back(texture2D());
	std::list<texture2D>::iterator lastElement = --(textureList.end());
	if(!(lastElement->loadTextureFile(path))) return false;

	inOutTexPtr = &(*lastElement);
	return true;
}

bool scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation)
{
	vertexGeometry* geomPtr;
	material* mtlPtr;
	if(!createVertexGeometry(geomPtr)) return false;
	if(!createMaterial(mtlPtr)) return false;

	scenegraph.push_back(staticSceneObject(id,position,geomPtr,mtlPtr));
	return true;
}

bool scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const char * const path, const bool flag)
{
	vertexGeometry* geomPtr;
	material* mtlPtr;

	if(flag==0)
	{
		if(!createVertexGeometry(path,geomPtr)) return false;
		if(!createMaterial(mtlPtr)) return false;
	}
	else if(flag==1)
	{
		if(!createVertexGeometry(geomPtr)) return false;
		if(!createMaterial(path,mtlPtr)) return false;
	}
	else
	{
		return false;
	}

	scenegraph.push_back(staticSceneObject(id,position,geomPtr,mtlPtr));
	return true;
}

bool scene::createStaticSceneObject(const int id, const glm::vec3 position, const glm::quat orientation, const char * const geometryPath, const char * const materialPath)
{
	vertexGeometry* geomPtr;
	material* mtlPtr;
	if(!createVertexGeometry(geometryPath,geomPtr)) return false;
	if(!createMaterial(materialPath,mtlPtr)) return false;

	scenegraph.push_back(staticSceneObject(id,position,geomPtr,mtlPtr));
	return true;
}

bool scene::createSceneLight(const int id, const glm::vec3 position, glm::vec4 colour)
{
	lightSourceList.push_back(sceneLightSource(id, position, colour));
	return true;
}

bool scene::createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientation, float aspect, float fov)
{
	cameraList.push_back(sceneCamera(id, position, orientation, aspect, fov));
	return true;
}

void scene::setActiveCamera(const int inId)
{
	//	check camera list
	for(std::list<sceneCamera>::iterator i = cameraList.begin(); i != cameraList.end(); ++i)
	{
		if((i->getId())==inId)
		{
			activeCamera = &(*i);
		}
	}
}

void scene::testing()
{
	//	Ehhh, a few lines for testing
	activeCamera->rotate(45.0,glm::vec3(0.0,1.0,0.0));
	activeCamera->rotate(-45.0,glm::vec3(1.0,0.0,0.0));
	
	//scenegraph.begin()->rotate(180,glm::vec3(0.0,1.0,0.0));
	
	//std::cout<<textureList.size();
	//std::cout<<vboList.size()<<"\n";
	//std::cout<<scenegraph.size();

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
void scene::render()
{
	//	obtain transformation matrices
	glm::mat4 modelViewMx;
	glm::mat4 modelViewProjectionMx;
	glm::mat3 normalMx;

	glm::mat4 modelMx;
	glm::mat4 viewMx(activeCamera->computeViewMatrix());
	glm::mat4 projectionMx(activeCamera->computeProjectionMatrix(0.01f,100.0f));

	////
	//	access each entity of the scene and draw it
	////
	if(materialList.size() > 0)
	{
	GLSLProgram* currentPrgm(&(*shaderProgramList.begin()));
	material* currentMtl(&(*materialList.begin()));
	currentPrgm->use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	currentPrgm->setUniform("diffuseMap",0);
	currentMtl->getDiffuseMap()->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	currentPrgm->setUniform("specularMap",1);
	currentMtl->getSpecularMap()->bindTexture();
	glActiveTexture(GL_TEXTURE2);
	currentPrgm->setUniform("normalMap",2);
	currentMtl->getNormalMap()->bindTexture();

	for(std::list<staticSceneObject>::iterator i = scenegraph.begin(); i != scenegraph.end(); ++i)
	{
		modelMx = i->computeModelMatrix();
		modelViewMx = viewMx * modelMx;
		normalMx = glm::transpose(glm::inverse(glm::mat3(modelViewMx)));
		modelViewProjectionMx = projectionMx * viewMx * modelMx;

		//currentPrgm = (i->getMaterial())->getShaderProgram();
		//currentMtl = i->getMaterial();

		//currentPrgm->use();

		currentPrgm->setUniform("normalMatrix",normalMx);
		currentPrgm->setUniform("viewMatrix", viewMx);
		currentPrgm->setUniform("modelViewMatrix",modelViewMx);
		currentPrgm->setUniform("modelViewProjectionMatrix",modelViewProjectionMx);
		currentPrgm->setUniform("lightPosition",(lightSourceList.begin())->getPosition());
		currentPrgm->setUniform("lightColour",(lightSourceList.begin())->getColour());

		//glEnable(GL_TEXTURE_2D);
		//glActiveTexture(GL_TEXTURE0);
		//currentPrgm->setUniform("diffuseMap",0);
		//currentMtl->getDiffuseMap()->bindTexture();
		//glActiveTexture(GL_TEXTURE1);
		//currentPrgm->setUniform("specularMap",1);
		//currentMtl->getSpecularMap()->bindTexture();
		//glActiveTexture(GL_TEXTURE2);
		//currentPrgm->setUniform("normalMap",2);
		//currentMtl->getNormalMap()->bindTexture();

		(i->getGeometry())->draw(GL_TRIANGLES,36,0);
		i->rotate(0.1f,glm::vec3(0.0f,1.0f,0.0f));
	}
	}
}
