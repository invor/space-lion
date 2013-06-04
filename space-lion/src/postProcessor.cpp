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
	if(!gaussianShaderPrg.initShaders(GAUSSIAN)) return false;
	if(!gradientShaderPrg.initShaders(GRADIENT)) return false;
	if(!hesseShaderPrg.initShaders(HESSE)) return false;

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

void postProcessor::applyGaussian(framebufferObject *inputFbo, framebufferObject *targetFbo, float sigma, int stencilRadius)
{
	gaussianShaderPrg.use();

	/*	set uniform values that aren't influced by vertical/horizontal */
	gaussianShaderPrg.setUniform("stencilRadius", stencilRadius);
	gaussianShaderPrg.setUniform("sigma", sigma);

	/*	use the internal framebuffer B for the horizontal part of the seperated gaussian */
	B.bind();
	glViewport(0,0,B.getWidth(),B.getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gaussianShaderPrg.setUniform("pixelOffset", glm::vec2(1.0f/(inputFbo->getWidth()),0.0f));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	gaussianShaderPrg.setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);
	renderPlane.draw(GL_TRIANGLES,6,0);

	/*	switch rendering to the input frambuffer for the second, vertical filtering step*/
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gaussianShaderPrg.setUniform("pixelOffset", glm::vec2(0.0f,1.0f/inputFbo->getHeight()));
	glActiveTexture(GL_TEXTURE0);
	gaussianShaderPrg.setUniform("inputImage",0);
	B.bindColorbuffer(0);
	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::computeGradient(framebufferObject *inputFbo, framebufferObject *targetFbo)
{
	gradientShaderPrg.use();
	
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gradientShaderPrg.setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(),1.0f/inputFbo->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	gradientShaderPrg.setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void postProcessor::computeHesse(framebufferObject *inputFbo, framebufferObject *targetFbo)
{
	hesseShaderPrg.use();

	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gradientShaderPrg.setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(),1.0f/inputFbo->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	gradientShaderPrg.setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

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

void postProcessor::FBOToFBO(framebufferObject *inputFbo)
{
	idleShaderPrg.use();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	idleShaderPrg.setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw(GL_TRIANGLES,6,0);
}