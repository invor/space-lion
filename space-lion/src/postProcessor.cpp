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
	if(!poissonShaderPrg.initShaders(POISSON)) return false;
	if(!idleShaderPrg.initShaders(IDLE)) return false;
	if(!stampShaderPrg.initShaders(STAMP)) return false;

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
	currentFrame->bindColorbuffer();

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h)
{
	stampShaderPrg.use();
	stampShaderPrg.setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	stampShaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	glActiveTexture(GL_TEXTURE1);
	stampShaderPrg.setUniform("mask",1);
	glBindTexture(GL_TEXTURE_2D, mask);

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
	inputFBO->bindColorbuffer();

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound)
{
	poissonShaderPrg.use();
	poissonShaderPrg.setUniform("lowerBound", lowerBound);
	poissonShaderPrg.setUniform("upperBound", upperBound);

	glEnable(GL_TEXTURE_2D);

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		poissonShaderPrg.setUniform("inputImage",0);
		currentFrame->bindColorbuffer();
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg.setUniform("previousFrame",1);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		poissonShaderPrg.setUniform("inputImage",0);
		B.bindColorbuffer();
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg.setUniform("previousFrame",1);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;
	}
}

void postProcessor::applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, GLuint mask)
{
	poissonShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	poissonShaderPrg.setUniform("mask",0);
	glBindTexture(GL_TEXTURE_2D, mask);

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg.setUniform("inputImage",1);
		currentFrame->bindColorbuffer();
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg.setUniform("previousFrame",2);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg.setUniform("inputImage",1);
		B.bindColorbuffer();
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg.setUniform("previousFrame",2);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

	}
}