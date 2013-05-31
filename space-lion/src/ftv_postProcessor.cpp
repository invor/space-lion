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

	/*
	/	Prepare the intermediate framebuffer B for rendering
	*/
	B.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

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

void ftv_postProcessor::applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, framebufferObject* mask)
{
	poissonShaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	poissonShaderPrg.setUniform("mask",0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	poissonShaderPrg.setUniform("distanceMap",1);
	mask->bindColorbuffer(1);

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
		glActiveTexture(GL_TEXTURE3);
		poissonShaderPrg.setUniform("previousFrame",3);
		previousFrame->bindColorbuffer(0);
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
		glActiveTexture(GL_TEXTURE3);
		poissonShaderPrg.setUniform("previousFrame",3);
		previousFrame->bindColorbuffer(0);
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