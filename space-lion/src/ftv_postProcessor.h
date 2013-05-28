/*
---------------------------------------------------------------------------------------------------
File: ftv_postProcessor.h
Author: Michael Becher
Date of (presumingly) last edit: 29.05.2013

This C++ class is developed in the scope of FeTol at University Stuttgart (VISUS).
http://www.vis.uni-stuttgart.de/en/projects/fetol.html

Describtion: Extends the postProcessor class. Currently, this class contains all
functionality for my image space, fault tolerant visualization (ftv) techniques.
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
	ftv_postProcessor() : B(0,0,false,false), iterationCounter(1.0) {}
	~ftv_postProcessor() {}

	ftv_postProcessor(int w, int h) : B(w,h,true,false), iterationCounter(1.0) {}

	bool ftv_init();

	/*
	/	Generate a distance map for a given mask.
	*/
	void generateDistanceMap(GLuint mask, int w, int h);

	/*
	/	Generate a mask, indicating faulty regions, for fault tolerant visualization.
	/	Takes a float array as input, with every group of four floats denoting the lower left and upper right corner
	/	of a rectangular inpainting region.
	*/
	void generateFtvMask(GLuint regionsTexture, float w);

	/*
	/	Render the input texture to the currently bound framebuffer using a mask.
	/	Pixels covered by black areas of the mask are rendered black.
	*/
	void applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h);
	void applyMaskToImageToFBO(GLuint inputImage, framebufferObject* maskFbo, int w, int h);

	/*
	/	Apply poisson image editing to a region given by a rectangle.
	*/
	//void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound);

	/*
	/	Apply poisson image editing to [a] region[s] given by an image-mask.
	*/
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, framebufferObject* mask);

	/*
	/	Apply image inpainting to [a] region[s] given by an image-mask.
	*/
	void applyImageInpainting(framebufferObject *currentFrame, framebufferObject* mask, int iterations);

private:
	framebufferObject B;

	float iterationCounter;

	GLSLProgram poissonShaderPrg;
	GLSLProgram inpaintingShaderPrg;
	GLSLProgram stampShaderPrg;
	GLSLProgram distanceShaderPrg;
	GLSLProgram maskCreationShaderPrg;
};

#endif
