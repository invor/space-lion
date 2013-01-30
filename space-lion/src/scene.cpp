#include "scene.h"


scene::scene(void)
{
	lastTextureId = 10000;
}

scene::~scene(void)
{
}

void scene::render()
{
}

vertexGeometry* scene::createVertexGeometry()
{
	//	check list of vertexBufferObjects for default box object(id=0)
	for(std::list<vertexGeometry>::iterator i = vboList.begin(); i != vboList.end(); ++i)
	{
		if((i->getId())==0){
			return &(*i);
		}
	}

	//	if default box not already in list, continue here
	vertex3 *vertexArray = new vertex3[8];
	GLubyte *indexArray = new GLubyte[10];

	vertexArray[0]=vertex3(-0.5,-0.5,-0.5);vertexArray[1]=vertex3(0.5,-0.5,-0.5);
	vertexArray[2]=vertex3(0.5,-0.5,0.5);vertexArray[3]=vertex3(-0.5,-0.5,0.5);
	vertexArray[4]=vertex3(-0.5,0.5,-0.5);vertexArray[5]=vertex3(0.5,0.5,-0.5);
	vertexArray[6]=vertex3(0.5,0.5,0.5);vertexArray[7]=vertex3(-0.5,0.5,0.5);

	indexArray[0]=4;indexArray[1]=3;indexArray[2]=7;indexArray[3]=8;
	indexArray[4]=5;indexArray[5]=3;indexArray[6]=1;indexArray[7]=4;
	indexArray[8]=2;indexArray[9]=7;indexArray[10]=6;indexArray[11]=5;
	indexArray[12]=2;indexArray[13]=1;

	vboList.push_back(vertexGeometry(0));
	std::list<vertexGeometry>::iterator lastElement = --(vboList.end());
	lastElement->bufferDataFromArray(vertexArray,indexArray);

	return &(*lastElement);
}

material* scene::createMaterial()
{
	//	check list of materials for default material(id=0)
	for(std::list<material>::iterator i = materialList.begin(); i != materialList.end(); ++i)
	{
		if((i->getId())==0){
			return &*i;
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
	
	materialList.push_back(material(0,createShaderProgram(PHONG),createTexture(1,1,diffuseData),createTexture(1,1,specularData),createTexture(1,1,normalData)));
}

GLSLProgram* scene::createShaderProgram(shaderType type)
{
	//	check list of shader programs for the shader type
	for(std::list<GLSLProgram>::iterator i = shaderProgramList.begin(); i != shaderProgramList.end(); ++i)
	{
		if((i->getType())==type){
			return &*i;
		}
	}

	//	create a shader program object of specified type
	switch(type)
	{
	case PHONG :
		GLSLProgram shaderPrg;
		shaderPrg.compileShaderFromFile("..\..\space-lion\src\shader\v_phong.glsl",GL_VERTEX_SHADER);
		shaderPrg.compileShaderFromFile("..\..\space-lion\src\shader\f_phong.glsl",GL_VERTEX_SHADER);

		shaderPrg.link();
	}
}

texture* scene::createTexture(int dimX, int dimY, float* data)
{
	++lastTextureId;
	textureList.push_back(texture(lastTextureId));
	std::list<texture>::iterator lastElement = --(textureList.end());
	lastElement->load(dimX, dimY, data);

	return &(*lastElement);
}