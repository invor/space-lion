#include "ftv_postProcessor.h"

bool ftv_postProcessor::ftv_init()
{
	init();
	
	/*
	/	Load all ftv post processing shaders
	*/
	if(!poissonShaderPrg.initShaders(POISSON)) return false;
	if(!inpaintingShaderPrg.initShaders(INPAINTING)) return false;
	if(!stampShaderPrg.initShaders(STAMP)) return false;
	if(!distanceShaderPrg.initShaders(DISTANCEMAPPING)) return false;
	if(!maskCreationShaderPrg.initShaders(FTV_MASK)) return false;
	if(!coherenceShaderPrg.initShaders(COHERENCE)) return false;
	if(!improvedInpaintingShaderPrg.initShaders(IMPROVED_INPAINTING)) return false;
	if(!ftvGaussianShaderPrg.initShaders(FTV_GAUSSIAN)) return false;

	/*	The FBOs used for image inpainting require different color attachments */
	gaussianFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	gradientFbo.createColorAttachment(GL_RG32F,GL_RG,GL_FLOAT);
	hesseFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	coherenceFbo.createColorAttachment(GL_RGB32F,GL_RGB,GL_FLOAT);

	return true;
}

void ftv_postProcessor::generateDistanceMap(GLuint mask, int w, int h)
{
	distanceShaderPrg.use();
	distanceShaderPrg.setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	distanceShaderPrg.setUniform("mask",0);
	glBindTexture(GL_TEXTURE_2D, mask);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void ftv_postProcessor::generateFtvMask(GLuint regionsTexture, float w)
{
	maskCreationShaderPrg.use();
	maskCreationShaderPrg.setUniform("regionCount", w);

	glEnable(GL_TEXTURE_1D);
	glActiveTexture(GL_TEXTURE0);
	maskCreationShaderPrg.setUniform("inpaintingRegions", 0);
	glBindTexture(GL_TEXTURE_1D, regionsTexture);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void ftv_postProcessor::applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h)
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

void ftv_postProcessor::applyMaskToImageToFBO(GLuint inputImage, framebufferObject* maskFbo, int w, int h)
{
	stampShaderPrg.use();
	stampShaderPrg.setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	stampShaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	glActiveTexture(GL_TEXTURE1);
	stampShaderPrg.setUniform("mask",1);
	maskFbo->bindColorbuffer(0);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void ftv_postProcessor::applyFtvGaussian(framebufferObject *inputFbo, framebufferObject *targetFbo, framebufferObject *maskFbo, float sigma, int stencilRadius)
{
	ftvGaussianShaderPrg.use();

	/*	set uniform values that aren't influced by vertical/horizontal switch */
	ftvGaussianShaderPrg.setUniform("stencilRadius", stencilRadius);
	ftvGaussianShaderPrg.setUniform("sigma", sigma);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	ftvGaussianShaderPrg.setUniform("maskImage",0);
	maskFbo->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	ftvGaussianShaderPrg.setUniform("distanceMap",1);
	maskFbo->bindColorbuffer(1);

	/*	use the internal framebuffer for the horizontal part of the seperated gaussian */
	gaussianBackBuffer.bind();
	glViewport(0,0,gaussianBackBuffer.getWidth(),gaussianBackBuffer.getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ftvGaussianShaderPrg.setUniform("pixelOffset", glm::vec2(1.0f/(gaussianBackBuffer.getWidth()),0.0f));
	glActiveTexture(GL_TEXTURE2);
	ftvGaussianShaderPrg.setUniform("inputImage",2);
	inputFbo->bindColorbuffer(0);
	renderPlane.draw(GL_TRIANGLES,6,0);

	/*	switch rendering to the input frambuffer for the second, vertical filtering step*/
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ftvGaussianShaderPrg.setUniform("pixelOffset", glm::vec2(0.0f,1.0f/inputFbo->getHeight()));
	glActiveTexture(GL_TEXTURE2);
	ftvGaussianShaderPrg.setUniform("inputImage",2);
	gaussianBackBuffer.bindColorbuffer(0);
	renderPlane.draw(GL_TRIANGLES,6,0);
}

/*
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
*/

void ftv_postProcessor::applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, framebufferObject* mask, int iterations, int mode)
{
	poissonShaderPrg.use();

	poissonShaderPrg.setUniform("mode",mode);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	poissonShaderPrg.setUniform("mask",0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	poissonShaderPrg.setUniform("distanceMap",1);
	mask->bindColorbuffer(1);
	glActiveTexture(GL_TEXTURE3);
	poissonShaderPrg.setUniform("previousFrame",3);
	previousFrame->bindColorbuffer(0);

	iterationCounter = 1.0f;

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg.setUniform("inputImage",2);
		currentFrame->bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg.setUniform("iteration", iterationCounter);
		poissonShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg.setUniform("inputImage",2);
		B.bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;
	}
}

void ftv_postProcessor::applyImageInpainting(framebufferObject *currentFrame, framebufferObject* mask, int iterations)
{
	inpaintingShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	inpaintingShaderPrg.setUniform("mask",0);
	mask->bindColorbuffer(0);

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg.setUniform("iteration", iterationCounter);
		inpaintingShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg.setUniform("inputImage",1);
		currentFrame->bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg.setUniform("iteration", iterationCounter);
		inpaintingShaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg.setUniform("inputImage",1);
		B.bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;
	}
}

void ftv_postProcessor::applyImprovedImageInpainting(framebufferObject *inputFbo, framebufferObject* mask, int iterations)
{
	for(int i=0; i<iterations; i++)
	{
		/*	Compute the coherence flow field and strength */
		applyFtvGaussian(inputFbo,&gaussianFbo,mask,1.5f,1);
		computeGradient(&gaussianFbo,&gradientFbo);
		applyFtvGaussian(&gradientFbo,&gaussianFbo,mask,1.5f,1);
		computeHesse(&gaussianFbo,&hesseFbo);
		applyFtvGaussian(&hesseFbo,&hesseFbo,mask,1.5f,5);
		computeCoherence(&hesseFbo,&coherenceFbo);
	
		improvedInpaintingShaderPrg.use();
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		improvedInpaintingShaderPrg.setUniform("stencilSize",5.0f);
		improvedInpaintingShaderPrg.setUniform("h", glm::vec2(1.0f/B.getWidth(), 1.0f/B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		improvedInpaintingShaderPrg.setUniform("mask",0);
		mask->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		improvedInpaintingShaderPrg.setUniform("inputImage",1);
		inputFbo->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		improvedInpaintingShaderPrg.setUniform("coherenceImage",2);
		coherenceFbo.bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;
		

		/*	Compute the coherence flow field and strength */
		applyFtvGaussian(&B, &gaussianFbo,mask,1.5f,1);
		computeGradient(&gaussianFbo,&gradientFbo);
		applyFtvGaussian(&gradientFbo,&gaussianFbo,mask,1.5f,1);
		computeHesse(&gaussianFbo,&hesseFbo);
		applyFtvGaussian(&hesseFbo,&hesseFbo,mask,1.5f,5);
		computeCoherence(&hesseFbo,&coherenceFbo);

		improvedInpaintingShaderPrg.use();
		inputFbo->bind();
		glViewport(0,0,inputFbo->getWidth(),inputFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		improvedInpaintingShaderPrg.setUniform("stencilSize",5.0f);
		improvedInpaintingShaderPrg.setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(), 1.0f/inputFbo->getHeight()));
		glActiveTexture(GL_TEXTURE0);
		improvedInpaintingShaderPrg.setUniform("mask",0);
		mask->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		improvedInpaintingShaderPrg.setUniform("inputImage",1);
		B.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		improvedInpaintingShaderPrg.setUniform("coherenceImage",2);
		coherenceFbo.bindColorbuffer(0);
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

	}
	
}

void ftv_postProcessor::computeCoherence(framebufferObject *inputFbo, framebufferObject *coherenceFbo)
{
	coherenceShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	coherenceShaderPrg.setUniform("structureTensor",0);
	inputFbo->bindColorbuffer(0);

	coherenceFbo->bind();
	glViewport(0,0,coherenceFbo->getWidth(),coherenceFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderPlane.draw(GL_TRIANGLES,6,0);
}