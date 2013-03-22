#include "abstractPostProcessor.h"

abstractPostProcessor::abstractPostProcessor()
{
}

abstractPostProcessor::~abstractPostProcessor()
{
}

bool abstractPostProcessor::init()
{
	if(!initRenderPlane()) return false;
	if(!initShaderProgram()) return false;

	return true;
}

bool abstractPostProcessor::initRenderPlane()
{
	vertex5 *vertexArray = new vertex5[4];
	GLubyte *indexArray = new GLubyte[6];

	//	front face
	vertexArray[0]=vertex5(-1.0,-1.0,0.0,0.0,0.0);vertexArray[1]=vertex5(-1.0,1.0,0.0,0.0,1.0);
	vertexArray[2]=vertex5(1.0,1.0,0.0,1.0,1.0);vertexArray[3]=vertex5(1.0,-1.0,0.0,1.0,0.0);
	

	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;

	if(!(renderPlane.bufferDataFromArray(vertexArray,indexArray,sizeof(vertex5)*4,sizeof(GLubyte)*6))) return false;
	renderPlane.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex5),0);
	renderPlane.setVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(vertex5),(GLvoid*) sizeof(vertex3));

	return true;
}