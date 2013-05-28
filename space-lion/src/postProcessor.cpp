#include "postProcessor.h"

bool postProcessor::init()
{
	/*
	/	Create vertex geometry of the render plane
	*/
	vertex_pu *vertexArray = new vertex_pu[4];
	GLubyte *indexArray = new GLubyte[6];

	vertexArray[0]=vertex_pu(-1.0,-1.0,0.0,0.0,0.0);vertexArray[1]=vertex_pu(-1.0,1.0,0.0,0.0,1.0);
	vertexArray[2]=vertex_pu(1.0,1.0,0.0,1.0,1.0);vertexArray[3]=vertex_pu(1.0,-1.0,0.0,1.0,0.0);
	
	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;

	if(!(renderPlane.bufferDataFromArray(vertexArray,indexArray,sizeof(vertex_pu)*4,sizeof(GLubyte)*6))) return false;
	renderPlane.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertex_pu),0);
	renderPlane.setVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(vertex_pu),(GLvoid*) sizeof(vertex_p));

	/*
	/	Load all post processing shaders
	*/
	if(!fxaaShaderPrg.initShaders(FXAA)) return false;
	if(!idleShaderPrg.initShaders(IDLE)) return false;

	return true;
}

void postProcessor::applyFxaa(GLuint inputImage)
{
	fxaaShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	fxaaShaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::applyFxaa(framebufferObject *currentFrame)
{
	fxaaShaderPrg.use();
	fxaaShaderPrg.setUniform("imgDim", glm::vec2(currentFrame->getWidth(), currentFrame->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	fxaaShaderPrg.setUniform("inputImage",0);
	currentFrame->bindColorbuffer(0);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::imageToFBO(GLuint inputImage)
{
	idleShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	idleShaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::FBOToFBO(framebufferObject *inputFBO)
{
	idleShaderPrg.use();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	idleShaderPrg.setUniform("inputImage",0);
	inputFBO->bindColorbuffer(0);

	renderPlane.draw(GL_TRIANGLES,6,0);
}