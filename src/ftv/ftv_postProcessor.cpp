#include "ftv_postProcessor.h"

bool Ftv_PostProcessor::ftv_init(Ftv_ResourceManager *ftv_resource_mngr)
{
	if (!init(ftv_resource_mngr)) return false;

	/*	Create grid ibfvGrid */
	int subdivision = 100;
	float tileOffset = 2.0/(float)subdivision;
	float localOffset = 1.0/(float)subdivision;
	float posX = -1.0+localOffset;
	float posY = -1.0+localOffset;
	int vertexCount = subdivision*subdivision*6;

	Vertex_pu *vertexArray = new Vertex_pu[vertexCount];
	GLuint *indexArray = new GLuint[vertexCount];

	/*
	/	A bit of magic.
	*/
	int vertexTileOffset;
	for(int i=0;i<subdivision;i++)
	{
		for(int j=0;j<subdivision;j++)
		{
			vertexTileOffset = ((i*subdivision*6)+j*6);
			//std::cout<<localOffset<<" "<<posX<<" "<<posY<<std::endl;
			vertexArray[vertexTileOffset]=Vertex_pu(posX-localOffset,posY-localOffset,0.0,((posX-localOffset)*0.5)+0.5,((posY-localOffset)*0.5)+0.5);
			vertexArray[vertexTileOffset+1]=Vertex_pu(posX+localOffset,posY+localOffset,0.0,((posX+localOffset)*0.5)+0.5,((posY+localOffset)*0.5)+0.5);
			vertexArray[vertexTileOffset+2]=Vertex_pu(posX-localOffset,posY+localOffset,0.0,((posX-localOffset)*0.5)+0.5,((posY+localOffset)*0.5)+0.5);

			vertexArray[vertexTileOffset+3]=Vertex_pu(posX+localOffset,posY+localOffset,0.0,((posX+localOffset)*0.5)+0.5,((posY+localOffset)*0.5)+0.5);
			vertexArray[vertexTileOffset+4]=Vertex_pu(posX-localOffset,posY-localOffset,0.0,((posX-localOffset)*0.5)+0.5,((posY-localOffset)*0.5)+0.5);
			vertexArray[vertexTileOffset+5]=Vertex_pu(posX+localOffset,posY-localOffset,0.0,((posX+localOffset)*0.5)+0.5,((posY-localOffset)*0.5)+0.5);
			
			indexArray[vertexTileOffset+0]=vertexTileOffset+0;
			indexArray[vertexTileOffset+1]=vertexTileOffset+1;
			indexArray[vertexTileOffset+2]=vertexTileOffset+2;
			indexArray[vertexTileOffset+3]=vertexTileOffset+3;
			indexArray[vertexTileOffset+4]=vertexTileOffset+4;
			indexArray[vertexTileOffset+5]=vertexTileOffset+5;

			posX += tileOffset;
		}
		posX = -1.0+localOffset;
		posY += tileOffset;
	}

	if(!(ibfvGrid.bufferDataFromArray(vertexArray,indexArray,sizeof(Vertex_pu)*vertexCount,sizeof(GLuint)*vertexCount,GL_TRIANGLES))) return false;
	ibfvGrid.setVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),0);
	ibfvGrid.setVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex_pu),(GLvoid*) sizeof(Vertex_p));


	/*	Load all ftv post processing shaders */
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_POISSON, poissonShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_LIC_INPAINTING, licInpaintingShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_INPAINTING, inpaintingShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(STAMP, stampShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(DISTANCEMAPPING, distanceShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_MASK, maskCreationShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(COHERENCE, coherenceShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_IMPROVED_INPAINTING, improvedInpaintingShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_GAUSSIAN, ftvGaussianShaderPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_MASK_SHRINK, shrinkMaskPrg)) return false;
	if (!ftv_resource_mngr->createFtvShaderProgram(FTV_TEXTURE_ADVECTION, textureAdvectionPrg)) return false;

	/*	The FBOs used for image inpainting require different color attachments */
	gaussianFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	gradientFbo.createColorAttachment(GL_RG32F,GL_RG,GL_FLOAT);
	hesseFbo.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);
	coherenceFbo.createColorAttachment(GL_RGB32F,GL_RGB,GL_FLOAT);
	maskFboB.createColorAttachment(GL_RG32F,GL_RG,GL_FLOAT);
	maskFboB.createColorAttachment(GL_RGBA32F,GL_RGBA,GL_FLOAT);

	return true;
}

void Ftv_PostProcessor::generateDistanceMap(GLuint mask, int w, int h)
{
	distanceShaderPrg->use();
	distanceShaderPrg->setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	distanceShaderPrg->setUniform("mask",0);
	glBindTexture(GL_TEXTURE_2D, mask);

	renderPlane.draw();
}

void Ftv_PostProcessor::generateFtvMask(FramebufferObject *targetFbo, GLuint regionsTexture, float w)
{
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	maskCreationShaderPrg->use();
	maskCreationShaderPrg->setUniform("regionCount", w);
	maskCreationShaderPrg->setUniform("h", glm::vec2(1.0f/targetFbo->getWidth(), 1.0f/targetFbo->getHeight()));

	glEnable(GL_TEXTURE_1D);
	glActiveTexture(GL_TEXTURE0);
	maskCreationShaderPrg->setUniform("inpaintingRegions_tx1D", 0);
	glBindTexture(GL_TEXTURE_1D, regionsTexture);

	renderPlane.draw();
}

void Ftv_PostProcessor::shrinkFtvMask(FramebufferObject* targetFbo, FramebufferObject* mask)
{
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shrinkMaskPrg->use();
	shrinkMaskPrg->setUniform("h", glm::vec2(1.0f/targetFbo->getWidth(), 1.0f/targetFbo->getHeight()));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shrinkMaskPrg->setUniform("mask_tx2D", 0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	shrinkMaskPrg->setUniform("distance_tx2D", 1);
	mask->bindColorbuffer(1);

	renderPlane.draw();
}

void Ftv_PostProcessor::applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h)
{
	stampShaderPrg->use();
	stampShaderPrg->setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	stampShaderPrg->setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	glActiveTexture(GL_TEXTURE1);
	stampShaderPrg->setUniform("mask",1);
	glBindTexture(GL_TEXTURE_2D, mask);

	renderPlane.draw();
}

void Ftv_PostProcessor::applyMaskToImageToFBO(GLuint inputImage, FramebufferObject* maskFbo, int w, int h)
{
	stampShaderPrg->use();
	stampShaderPrg->setUniform("imgDim", glm::vec2(w, h));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	stampShaderPrg->setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	glActiveTexture(GL_TEXTURE1);
	stampShaderPrg->setUniform("mask",1);
	maskFbo->bindColorbuffer(0);

	renderPlane.draw();
}

void Ftv_PostProcessor::applyFtvGaussian(FramebufferObject *inputFbo, FramebufferObject *targetFbo, FramebufferObject *maskFbo, float sigma, int stencilRadius)
{
	ftvGaussianShaderPrg->use();

	/*	set uniform values that aren't influced by vertical/horizontal switch */
	ftvGaussianShaderPrg->setUniform("stencilRadius", stencilRadius);
	ftvGaussianShaderPrg->setUniform("sigma", sigma);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	ftvGaussianShaderPrg->setUniform("maskImage",0);
	maskFbo->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	ftvGaussianShaderPrg->setUniform("distanceMap",1);
	maskFbo->bindColorbuffer(1);

	/*	use the internal framebuffer for the horizontal part of the seperated gaussian */
	gaussianBackBuffer.bind();
	glViewport(0,0,gaussianBackBuffer.getWidth(),gaussianBackBuffer.getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ftvGaussianShaderPrg->setUniform("pixelOffset", glm::vec2(1.0f/(gaussianBackBuffer.getWidth()),0.0f));
	glActiveTexture(GL_TEXTURE2);
	ftvGaussianShaderPrg->setUniform("inputImage",2);
	inputFbo->bindColorbuffer(0);
	renderPlane.draw();

	/*	switch rendering to the input frambuffer for the second, vertical filtering step*/
	targetFbo->bind();
	glViewport(0,0,targetFbo->getWidth(),targetFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ftvGaussianShaderPrg->setUniform("pixelOffset", glm::vec2(0.0f,1.0f/inputFbo->getHeight()));
	glActiveTexture(GL_TEXTURE2);
	ftvGaussianShaderPrg->setUniform("inputImage",2);
	gaussianBackBuffer.bindColorbuffer(0);
	renderPlane.draw();
}

/*
void postProcessor::applyPoisson(FramebufferObject *currentFrame, FramebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound)
{
	poissonShaderPrg->use();
	poissonShaderPrg->setUniform("lowerBound", lowerBound);
	poissonShaderPrg->setUniform("upperBound", upperBound);

	glEnable(GL_TEXTURE_2D);

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		poissonShaderPrg->setUniform("inputImage",0);
		currentFrame->bindColorbuffer();
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg->setUniform("previousFrame",1);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		poissonShaderPrg->setUniform("inputImage",0);
		B.bindColorbuffer();
		glActiveTexture(GL_TEXTURE1);
		poissonShaderPrg->setUniform("previousFrame",1);
		previousFrame->bindColorbuffer();
		renderPlane.draw(GL_TRIANGLES,6,0);
		iterationCounter++;
	}
}
*/

void Ftv_PostProcessor::applyPoisson(FramebufferObject *currentFrame, FramebufferObject *previousFrame, FramebufferObject* mask, int iterations, int mode)
{
	poissonShaderPrg->use();

	poissonShaderPrg->setUniform("mode",mode);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	poissonShaderPrg->setUniform("mask_tx2D",0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	poissonShaderPrg->setUniform("distance_tx2D",1);
	mask->bindColorbuffer(1);
	glActiveTexture(GL_TEXTURE3);
	poissonShaderPrg->setUniform("prevFrame_tx2D",3);
	previousFrame->bindColorbuffer(0);

	iterationCounter = 1.0f;

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("h", glm::vec2(1.0/B.getWidth(), 1.0/B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg->setUniform("currFrame_tx2D",2);
		currentFrame->bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("h", glm::vec2(1.0/B.getWidth(), 1.0/B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg->setUniform("currFrame_tx2D",2);
		B.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
	}
}

void Ftv_PostProcessor::applyGuidedPoisson(FramebufferObject *currentFrame, GLuint guidanceField, FramebufferObject* mask, int iterations)
{
	poissonShaderPrg->use();

	poissonShaderPrg->setUniform("mode",2);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	poissonShaderPrg->setUniform("mask_tx2D",0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	poissonShaderPrg->setUniform("distance_tx2D",1);
	mask->bindColorbuffer(1);
	glActiveTexture(GL_TEXTURE3);
	poissonShaderPrg->setUniform("guidanceField_tx2D",3);
	glBindTexture(GL_TEXTURE_2D,guidanceField);

	iterationCounter = 1.0f;

	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("h", glm::vec2(1.0/B.getWidth(), 1.0/B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg->setUniform("currFrame_tx2D",2);
		currentFrame->bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		poissonShaderPrg->setUniform("iteration", iterationCounter);
		poissonShaderPrg->setUniform("h", glm::vec2(1.0/B.getWidth(), 1.0/B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		poissonShaderPrg->setUniform("currFrame_tx2D",2);
		B.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
	}
}

//	void Ftv_PostProcessor::applyImageInpainting(FramebufferObject *currentFrame, FramebufferObject* mask, int iterations)
//	{
//		inpaintingShaderPrg->use();
//	
//		glEnable(GL_TEXTURE_2D);
//		glActiveTexture(GL_TEXTURE0);
//		inpaintingShaderPrg->setUniform("mask",0);
//		mask->bindColorbuffer(0);
//	
//		for(int i=0; i<iterations; i++)
//		{
//			B.bind();
//			glViewport(0,0,B.getWidth(),B.getHeight());
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			inpaintingShaderPrg->setUniform("iteration", iterationCounter);
//			inpaintingShaderPrg->setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
//			glActiveTexture(GL_TEXTURE1);
//			inpaintingShaderPrg->setUniform("inputImage",1);
//			currentFrame->bindColorbuffer(0);
//			renderPlane.draw(GL_TRIANGLES,6,0);
//			iterationCounter++;
//	
//			currentFrame->bind();
//			glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			inpaintingShaderPrg->setUniform("iteration", iterationCounter);
//			inpaintingShaderPrg->setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
//			glActiveTexture(GL_TEXTURE1);
//			inpaintingShaderPrg->setUniform("inputImage",1);
//			B.bindColorbuffer(0);
//			renderPlane.draw(GL_TRIANGLES,6,0);
//			iterationCounter++;
//		}
//	}

void Ftv_PostProcessor::applyLicInpainting(FramebufferObject *currentFrame, GLuint guidanceField, FramebufferObject* mask, int iterations)
{
	licInpaintingShaderPrg->use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	licInpaintingShaderPrg->setUniform("mask_tx2D", 0);
	mask->bindColorbuffer(0);
	glActiveTexture(GL_TEXTURE1);
	licInpaintingShaderPrg->setUniform("guidanceField_tx2D", 1);
	glBindTexture(GL_TEXTURE_2D, guidanceField);

	iterationCounter = 1.0f;

	for (int i = 0; i < iterations; i++)
	{
		B.bind();
		glViewport(0, 0, B.getWidth(), B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		licInpaintingShaderPrg->setUniform("h", glm::vec2(1.0 / B.getWidth(), 1.0 / B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		licInpaintingShaderPrg->setUniform("currFrame_tx2D", 2);
		currentFrame->bindColorbuffer(0);
		renderPlane.draw();

		currentFrame->bind();
		glViewport(0, 0, currentFrame->getWidth(), currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		licInpaintingShaderPrg->setUniform("h", glm::vec2(1.0 / B.getWidth(), 1.0 / B.getHeight()));
		glActiveTexture(GL_TEXTURE2);
		licInpaintingShaderPrg->setUniform("currFrame_tx2D", 2);
		B.bindColorbuffer(0);
		renderPlane.draw();
	}
}

void Ftv_PostProcessor::applyImageInpainting(FramebufferObject *inputFbo, FramebufferObject* mask, int iterations)
{
	for(int i=0; i<iterations; i++)
	{
		/*	Compute the gradient */
		applyFtvGaussian(inputFbo,&gaussianFbo,mask,1.5f,1);
		computeGradient(&gaussianFbo,&gradientFbo);
		
		inpaintingShaderPrg->use();
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg->setUniform("stencilSize",1.0f);
		inpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/B.getWidth(), 1.0f/B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		inpaintingShaderPrg->setUniform("mask_tx2D",0);
		mask->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg->setUniform("input_tx2D",1);
		inputFbo->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		inpaintingShaderPrg->setUniform("gradient_tx2D",2);
		gradientFbo.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(&maskFboB,mask);
		
		/*	Compute the gradient */
		applyFtvGaussian(&B, &gaussianFbo,&maskFboB,1.5f,1);
		computeGradient(&gaussianFbo,&gradientFbo);
		
		inpaintingShaderPrg->use();
		inputFbo->bind();
		glViewport(0,0,inputFbo->getWidth(),inputFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg->setUniform("stencilSize",1.0f);
		inpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(), 1.0f/inputFbo->getHeight()));
		glActiveTexture(GL_TEXTURE0);
		inpaintingShaderPrg->setUniform("mask_tx2D",0);
		maskFboB.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg->setUniform("input_tx2D",1);
		B.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		inpaintingShaderPrg->setUniform("gradient_tx2D",2);
		gradientFbo.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(mask,&maskFboB);
	}
}

void Ftv_PostProcessor::applyImprovedImageInpainting(FramebufferObject *inputFbo, FramebufferObject* mask, int iterations)
{
	for(int i=0; i<iterations; i++)
	{
		/*	Compute the coherence flow field and strength */
		//	applyFtvGaussian(inputFbo,&gaussianFbo,mask,1.5f,1);
		//	computeGradient(&gaussianFbo,&gradientFbo);
		//	applyFtvGaussian(&gradientFbo,&gaussianFbo,mask,1.5f,1);
		//	computeHesse(&gaussianFbo,&hesseFbo);
		//	applyFtvGaussian(&hesseFbo,&hesseFbo,mask,1.5f,2);
		//	computeCoherence(&hesseFbo,&coherenceFbo);

		/*	Temporary fix for the very wrong implementation above */
		applyFtvGaussian(inputFbo,&gaussianFbo,mask,1.4f,1);
		computeStructureTensor(&gaussianFbo,&hesseFbo);
		applyFtvGaussian(&hesseFbo,&hesseFbo,mask,1.5f,4);
		computeCoherence(&hesseFbo,&coherenceFbo);
	
		improvedInpaintingShaderPrg->use();
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		improvedInpaintingShaderPrg->setUniform("stencilSize",4.0f);
		improvedInpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/B.getWidth(), 1.0f/B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		improvedInpaintingShaderPrg->setUniform("mask_tx2D",0);
		mask->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		improvedInpaintingShaderPrg->setUniform("input_tx2D",1);
		inputFbo->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		improvedInpaintingShaderPrg->setUniform("coherence_tx2D",2);
		coherenceFbo.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(&maskFboB,mask);
		
		/*	Compute the coherence flow field and strength */
		//	applyFtvGaussian(&B, &gaussianFbo,&maskFboB,1.5f,1);
		//	computeGradient(&gaussianFbo,&gradientFbo);
		//	applyFtvGaussian(&gradientFbo,&gaussianFbo,&maskFboB,1.5f,1);
		//	computeHesse(&gaussianFbo,&hesseFbo);
		//	applyFtvGaussian(&hesseFbo,&hesseFbo,&maskFboB,1.5f,2);
		//	computeCoherence(&hesseFbo,&coherenceFbo);

		/*	Temporary fix for the very wrong implementation above */
		applyFtvGaussian(&B, &gaussianFbo,&maskFboB,1.4f,1);
		computeStructureTensor(&gaussianFbo,&hesseFbo);
		applyFtvGaussian(&hesseFbo,&hesseFbo,&maskFboB,1.5f,4);
		computeCoherence(&hesseFbo,&coherenceFbo);
		
		improvedInpaintingShaderPrg->use();
		inputFbo->bind();
		glViewport(0,0,inputFbo->getWidth(),inputFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		improvedInpaintingShaderPrg->setUniform("stencilSize",4.0f);
		improvedInpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(), 1.0f/inputFbo->getHeight()));
		glActiveTexture(GL_TEXTURE0);
		improvedInpaintingShaderPrg->setUniform("mask_tx2D",0);
		maskFboB.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		improvedInpaintingShaderPrg->setUniform("input_tx2D",1);
		B.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		improvedInpaintingShaderPrg->setUniform("coherence_tx2D",2);
		coherenceFbo.bindColorbuffer(0);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(mask,&maskFboB);
	}
	
}

void Ftv_PostProcessor::applyGuidedImageInpainting(FramebufferObject *inputFbo, FramebufferObject* mask, GLuint guidanceField , int iterations)
{
	for(int i=0; i<iterations; i++)
	{
		inpaintingShaderPrg->use();
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg->setUniform("stencilSize",1.0f);
		inpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/B.getWidth(), 1.0f/B.getHeight()));
		glActiveTexture(GL_TEXTURE0);
		inpaintingShaderPrg->setUniform("mask_tx2D",0);
		mask->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg->setUniform("input_tx2D",1);
		inputFbo->bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		inpaintingShaderPrg->setUniform("gradient_tx2D",2);
		glBindTexture(GL_TEXTURE_2D, guidanceField);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(&maskFboB,mask);
		
		inpaintingShaderPrg->use();
		inputFbo->bind();
		glViewport(0,0,inputFbo->getWidth(),inputFbo->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		inpaintingShaderPrg->setUniform("stencilSize",1.0f);
		inpaintingShaderPrg->setUniform("h", glm::vec2(1.0f/inputFbo->getWidth(), 1.0f/inputFbo->getHeight()));
		glActiveTexture(GL_TEXTURE0);
		inpaintingShaderPrg->setUniform("mask_tx2D",0);
		maskFboB.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE1);
		inpaintingShaderPrg->setUniform("input_tx2D",1);
		B.bindColorbuffer(0);
		glActiveTexture(GL_TEXTURE2);
		inpaintingShaderPrg->setUniform("gradient_tx2D",2);
		glBindTexture(GL_TEXTURE_2D, guidanceField);
		renderPlane.draw();
		iterationCounter++;
		
		/*	Shrink inpainting mask */
		shrinkFtvMask(mask,&maskFboB);
	}

}

void Ftv_PostProcessor::computeCoherence(FramebufferObject *inputFbo, FramebufferObject *coherenceFbo)
{
	coherenceShaderPrg->use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	coherenceShaderPrg->setUniform("structureTensor",0);
	inputFbo->bindColorbuffer(0);

	coherenceFbo->bind();
	glViewport(0,0,coherenceFbo->getWidth(),coherenceFbo->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderPlane.draw();
}

void Ftv_PostProcessor::textureAdvection(FramebufferObject *inputFbo, GLuint guidanceField)
{
	textureAdvectionPrg->use();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	textureAdvectionPrg->setUniform("inputImage",0);
	inputFbo->bindColorbuffer(0);
	glEnable(GL_TEXTURE_3D);
	glActiveTexture(GL_TEXTURE1);
	textureAdvectionPrg->setUniform("guidanceField_tx3D",1);
	glBindTexture(GL_TEXTURE_3D,guidanceField);

	ibfvGrid.draw();
}