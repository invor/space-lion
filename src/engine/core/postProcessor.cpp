#include "postProcessor.h"

bool PostProcessor::init(ResourceManager* resourceMngr)
{
	/*
	/	Create vertex geometry of the render plane
	*/
	Vertex_pu *vertexArray = new Vertex_pu[4];
	GLuint *indexArray = new GLuint[6];

	vertexArray[0]=Vertex_pu(-1.0,-1.0,0.0,0.0,0.0);vertexArray[1]=Vertex_pu(-1.0,1.0,0.0,0.0,1.0);
	vertexArray[2]=Vertex_pu(1.0,1.0,0.0,1.0,1.0);vertexArray[3]=Vertex_pu(1.0,-1.0,0.0,1.0,0.0);
	
	indexArray[0]=0;indexArray[1]=2;indexArray[2]=1;
	indexArray[3]=2;indexArray[4]=0;indexArray[5]=3;

	if(!(renderPlane.bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pu)*4,sizeof(GLuint)*6,GL_TRIANGLES))) return false;
	renderPlane.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),0);
	renderPlane.setVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),(GLvoid*) sizeof(Vertex_p));

	/*	Load all post processing shaders. */
	if( !resourceMngr->createShaderProgram(FXAA,fxaaShaderPrg) ) return false;
	if( !resourceMngr->createShaderProgram(IDLE,idleShaderPrg) ) return false;
	if( !resourceMngr->createShaderProgram(GAUSSIAN,gaussianShaderPrg) ) return false;
	if( !resourceMngr->createShaderProgram(GRADIENT,gradientShaderPrg) ) return false;
	if( !resourceMngr->createShaderProgram(HESSE,hesseShaderPrg) ) return false;
	if( !resourceMngr->createShaderProgram(STRUCTURE_TENSOR,structureTensorShaderPrg) ) return false;


	/*	Prepare the intermediate framebuffers for rendering */
	B.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	gaussianBackBuffer.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	return true;
}

void PostProcessor::applyFxaa(GLuint inputImage)
{
	fxaaShaderPrg->use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	fxaaShaderPrg->setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw();
}

void PostProcessor::applyFxaa(FramebufferObject *currentFrame)
{
	fxaaShaderPrg->use();
	fxaaShaderPrg->setUniform("imgDim", glm::vec2(currentFrame->getWidth(), currentFrame->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	fxaaShaderPrg->setUniform("inputImage",0);
	currentFrame->bindColorbuffer(0);

	renderPlane.draw();
}

void PostProcessor::applyGaussian(FramebufferObject *inputFbo, FramebufferObject *targetFbo, float sigma, int stencilRadius)
{
	gaussianShaderPrg->use();

	/*	set uniform values that aren't influced by vertical/horizontal */
	gaussianShaderPrg->setUniform("stencilRadius", stencilRadius);
	gaussianShaderPrg->setUniform("sigma", sigma);

	/*	use the internal framebuffer B for the horizontal part of the seperated gaussian */
	B.bind();
	glViewport(0,0,B.getWidth(),B.getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gaussianShaderPrg->setUniform("pixelOffset", glm::vec2(1.0f/(inputFbo->getWidth()),0.0f));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	gaussianShaderPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);
	renderPlane.draw();

	/*	switch rendering to the input frambuffer for the second, vertical filtering step*/
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gaussianShaderPrg->setUniform("pixelOffset", glm::vec2(0.0f,1.0f/inputFbo->getHeight()));
	glActiveTexture(GL_TEXTURE0);
	gaussianShaderPrg->setUniform("inputImage",0);
	B.bindColorbuffer(0);
	renderPlane.draw();
}

void PostProcessor::computeGradient(FramebufferObject *inputFbo, FramebufferObject *targetFbo)
{
	gradientShaderPrg->use();
	
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gradientShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(),1.0f/inputFbo->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	gradientShaderPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw();
}

void PostProcessor::computeHesse(FramebufferObject *inputFbo, FramebufferObject *targetFbo)
{
	hesseShaderPrg->use();

	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	hesseShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(),1.0f/inputFbo->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	hesseShaderPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw();
}

void PostProcessor::computeStructureTensor(FramebufferObject *inputFbo, FramebufferObject *targetFbo)
{
	structureTensorShaderPrg->use();

	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	structureTensorShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(),1.0f/inputFbo->getHeight()));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	structureTensorShaderPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw();
}

void PostProcessor::imageToFBO(GLuint inputImage)
{
	idleShaderPrg->use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	idleShaderPrg->setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw();
}

void PostProcessor::FBOToFBO(FramebufferObject *inputFbo)
{
	idleShaderPrg->use();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	idleShaderPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);

	renderPlane.draw();
}