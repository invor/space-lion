/*
---------------------------------------------------------------------------------------------------
File: ftv_postProcessor.h
Author: Michael Becher
Date of (presumingly) last edit: 29.05.2013

This C++ class is developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Description: Extends the postProcessor class. Currently, this class contains most of the
functionality for several image space fault tolerant visualization (ftv) techniques.
It offers handling and creation of image masks as well as the actual methods for
image inpainting and poisson image editing.
---------------------------------------------------------------------------------------------------
*/

#ifndef ftv_postProcessor_h
#define ftv_postProcessor_h

#include "postProcessor.h"

class ftv_postProcessor : public postProcessor
{
public:
	ftv_postProcessor() : postProcessor(), iterationCounter(1.0),
							gaussianFbo(0,0,false,false),
							gradientFbo(0,0,false,false),
							hesseFbo(0,0,false,false),
							coherenceFbo(0,0,false,false),
							maskFboB(0,0,false,false) {}
	~ftv_postProcessor() {}

	ftv_postProcessor(int w, int h) : postProcessor(w,h), iterationCounter(1.0),
							gaussianFbo(w,h,false,false),
							gradientFbo(w,h,false,false),
							hesseFbo(w,h,false,false),
							coherenceFbo(w,h,false,false),
							maskFboB(w,h,false,false) {}

	bool ftv_init(resourceManager *resourceMngr);

	/*
	/	Generate a distance map for a given mask.
	*/
	void generateDistanceMap(GLuint mask, int w, int h);

	/*
	/	Generate a mask, indicating faulty regions, for fault tolerant visualization.
	/	Takes a float array as input, with every group of four floats denoting the lower left and upper right corner
	/	of a rectangular inpainting region.
	*/
	void generateFtvMask(framebufferObject *targetFbo, GLuint regionsTexture, float w);

	void shrinkFtvMask(framebufferObject* targetFbo, framebufferObject* mask);

	/*
	/	Render the input texture to the currently bound framebuffer using a mask.
	/	Pixels covered by black areas of the mask are rendered black.
	*/
	void applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h);
	void applyMaskToImageToFBO(GLuint inputImage, framebufferObject* maskFbo, int w, int h);

	/*
	/	Applies a modified seperated gaussian the first color attachment of the input framebuffer.
	/	Takes inpainting regions into account during filtering -> ignores values from these regions.
	*/
	void applyFtvGaussian(framebufferObject *inputFbo, framebufferObject *targetFbo, framebufferObject *maskFbo, float sigma, int stencilRadius);

	/*
	/	Apply poisson image editing to a region given by a rectangle.
	*/
	//void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound);

	/*
	/	Apply poisson image editing to [a] region[s] given by an image-mask.
	*/
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, framebufferObject* mask, int iterations, int mode);

	void applyGuidedPoisson(framebufferObject *currentFrame, GLuint guidanceField, framebufferObject* mask, int iterations);

	/*
	/	Apply image inpainting to [a] region[s] given by an image-mask.
	*/
	void applyImageInpainting(framebufferObject *inputFbo, framebufferObject* mask, int iterations);

	/*
	/	Apply image inpainting to [a] region[s] given by an image-mask.
	/	Use precomputed coherence flow field and strength instead of computing gradients on the fly.
	*/
	void applyImprovedImageInpainting(framebufferObject *inputFbo, framebufferObject* mask, int iterations);

	/*
	/	Compute the coherence flow field and coherence strength.
	/	Takes the structure tensor as input.
	*/
	void computeCoherence(framebufferObject *inputFbo, framebufferObject *coherenceFbo);

private:

	float iterationCounter;

	GLSLProgram *poissonShaderPrg;
	GLSLProgram *inpaintingShaderPrg;
	GLSLProgram *stampShaderPrg;
	GLSLProgram *distanceShaderPrg;
	GLSLProgram *maskCreationShaderPrg;
	GLSLProgram *coherenceShaderPrg;
	GLSLProgram *improvedInpaintingShaderPrg;
	GLSLProgram *ftvGaussianShaderPrg;
	GLSLProgram *shrinkMaskPrg;

	/*	Some additional FBOs are required for coherence computations */
	framebufferObject gaussianFbo;
	framebufferObject gradientFbo;
	framebufferObject hesseFbo;
	framebufferObject coherenceFbo;
	framebufferObject maskFboB;
};

#endif
