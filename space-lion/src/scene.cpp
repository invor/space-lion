#include "scene.h"


scene::scene()
{
	lastTextureId = 10000;
}

scene::~scene()
{
}

bool scene::createVertexGeometry(vertexGeometry*& inOutGeomPtr)
{
	//	check list of vertexBufferObjects for default box object(filename="0")
	for(std::list<vertexGeometry>::iterator i = vboList.begin(); i != vboList.end(); ++i)
	{
		if((i->getFilename())=="0"){
			inOutGeomPtr = &(*i);
			return true;
		}
	}

	//	if default box not already in list, continue here
	vertex3 *vertexArray = new vertex15[24];
	GLubyte *indexArray = new GLubyte[36];

	//	front face
	vertexArray[0]=vertex15(-0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[1]=vertex15(-0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[2]=vertex15(0.5,0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[3]=vertex15(0.5,-0.5,0.5,0.0,0.0,1.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	right face
	vertexArray[4]=vertex15(0.5,-0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,0.0,0.0);vertexArray[5]=vertex15(0.5,0.5,0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,0.0,1.0);
	vertexArray[6]=vertex15(0.5,0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,1.0,1.0);vertexArray[7]=vertex15(0.5,-0.5,-0.5,1.0,0.0,0.0,0.0,0.0,-1.0,255,0,0,128,1.0,0.0);
	//	left face
	vertexArray[8]=vertex15(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,0.0,0.0);vertexArray[9]=vertex15(-0.5,0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,0.0,1.0);
	vertexArray[10]=vertex15(-0.5,0.5,0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,1.0,1.0);vertexArray[11]=vertex15(-0.5,-0.5,-0.5,-1.0,0.0,0.0,0.0,0.0,1.0,255,0,0,128,1.0,0.0);
	//	back face
	vertexArray[12]=vertex15(0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[13]=vertex15(0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[14]=vertex15(-0.5,0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[15]=vertex15(-0.5,-0.5,-0.5,0.0,0.0,-1.0,-1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	bottom face
	vertexArray[16]=vertex15(-0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[17]=vertex15(-0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[18]=vertex15(0.5,-0.5,-0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[19]=vertex15(0.5,-0.5,0.5,0.0,-1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);
	//	top face
	vertexArray[20]=vertex15(-0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,0.0);vertexArray[21]=vertex15(-0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,0.0,1.0);
	vertexArray[22]=vertex15(0.5,0.5,-0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,1.0);vertexArray[23]=vertex15(0.5,0.5,0.5,0.0,1.0,0.0,1.0,0.0,0.0,255,0,0,128,1.0,0.0);

	indexArray[0]=0;indexArray[1]=1;indexArray[2]=2;
	indexArray[3]=1;indexArray[4]=2;indexArray[5]=3;
	indexArray[6]=4;indexArray[7]=5;indexArray[8]=6;
	indexArray[9]=5;indexArray[10]=6;indexArray[11]=7;
	indexArray[12]=8;indexArray[13]=9;indexArray[14]=10;
	indexArray[15]=9;indexArray[16]=10;indexArray[17]=11;
	indexArray[18]=12;indexArray[19]=13;indexArray[20]=14;
	indexArray[21]=13;indexArray[22]=14;indexArray[23]=15;
	indexArray[24]=16;indexArray[25]=17;indexArray[26]=18;
	indexArray[27]=17;indexArray[28]=18;indexArray[29]=19;
	indexArray[30]=20;indexArray[31]=21;indexArray[32]=22;
	indexArray[33]=21;indexArray[34]=22;indexArray[35]=23;

	vboList.push_back(vertexGeometry(0));
	std::list<vertexGeometry>::iterator lastElement = --(vboList.end());
	if(!(lastElement->bufferDataFromArray(vertexArray,indexArray))) return false;
	lastElement->setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex15),0);
	lastElement->setVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(vertex15),(GLvoid*) sizeof(vertex3));
	lastElement->setVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(vertex15),(GLvoid*) sizeof(vertex6));
	lastElement->setVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_FALSE,sizeof(vertex15),(GLvoid*) sizeof(vertex9));
	lastElement->setVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,sizeof(vertex15),(GLvoid*) sizeof(vertex13));

	inOutGeomPtr = &(*lastElement);
	return true;
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
	normalData[0]=0.0f; normalData[1]=1.0f; normalData[2]=0.0f; normalData[3]=0.0f;
	
	GLSLProgram* prgPtr;
	texture* texPtr1;
	texture* texPtr2;
	texture* texPtr3;
	if(!createShaderProgram(PHONG,prgPtr)) return false;
	if(!createTexture(1,1,diffuseData,texPtr1)) return false;
	if(!createTexture(1,1,specularData,texPtr2)) return false;
	if(!createTexture(1,1,normalData,texPtr3)) return false;

	materialList.push_back(material(0,prgPtr,texPtr1,texPtr2,texPtr3));

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

	//	create a shader program object of specified type
	switch(type)
	{
	case PHONG :
		GLSLProgram shaderPrg(PHONG);
		//shaderPrg.compileShaderFromFile("../../space-lion/src/shader/v_phong.glsl",GL_VERTEX_SHADER);
		//shaderPrg.compileShaderFromFile("../../space-lion/src/shader/f_phong.glsl",GL_FRAGMENT_SHADER);
		if(!shaderPrg.compileShaderFromFile("v_phong.glsl",GL_VERTEX_SHADER)) return false;
		if(!shaderPrg.compileShaderFromFile("f_phong.glsl",GL_FRAGMENT_SHADER)) return false;
		shaderPrg.bindAttribLocation(0,"vPosition");
		shaderPrg.bindAttribLocation(1,"vNormal");
		shaderPrg.bindAttribLocation(2,"vTangent");
		shaderPrg.bindAttribLocation(3,"vColour");
		shaderPrg.bindAttribLocation(4,"vUVCoord");
		if(!shaderPrg.link()) return false;
		std::cout<<shaderPrg.getLog();
		glUseProgram(0);
		shaderProgramList.push_back(shaderPrg);
		std::list<GLSLProgram>::iterator lastElement = --(shaderProgramList.end());
		inOutPrgPtr = &(*lastElement);
		return true;
		break;
	}
	return false;
}

bool scene::createTexture(int dimX, int dimY, float* data, texture*& inOutTexPtr)
{
	++lastTextureId;
	//	somewhat messy, but will hopefully hold the code together for now
	char* tstr = "0";
	//_itoa(lastTextureId,tstr,10);
	textureList.push_back(texture(tstr));
	std::list<texture>::iterator lastElement = --(textureList.end());
	if(!(lastElement->load(dimX, dimY, data))) return false;

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

bool scene::createSceneLight(const int id, const glm::vec3 position, glm::vec4 colour)
{
	lightSourceList.push_back(sceneLightSource(id, position, colour));
}

bool scene::createSceneCamera(const int id, const glm::vec3 position, const glm::quat orientations, float aspect, float fov)
{
	cameraList.push_back(sceneCamera(id, position, orientations, aspect, fov));
}

glm::mat4 scene::computeModelMatrix(const glm::vec3 position, const glm::quat orientation)
{
	return glm::translate(glm::rotate(glm::mat4(1.0),orientation.w,glm::vec3(orientation.x,orientation.y,orientation.z)),position);
}

void scene::render()
{
	glClear( GL_COLOR_BUFFER_BIT );
	for(std::list<staticSceneObject>::iterator i = scenegraph.begin(); i != scenegraph.end(); ++i)
	{
		//	access each entity of the scene and draw it
	}
}