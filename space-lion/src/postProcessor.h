#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class postProcessor
{
public:
	postProcessor() : B(0,0,false,false), iterationCounter(1.0) {}
	~postProcessor() {}

	postProcessor(int w, int h) : B(w,h,true,false), iterationCounter(1.0) {}

	bool init();

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(framebufferObject *currentFrame);

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
	/	Render the input texture to the currently bound framebuffer.
	*/
	void imageToFBO(GLuint inputImage);

	/*
	/	Render the color attachment of the input framebuffer to the currently bound framebuffer.
	*/
	void FBOToFBO(framebufferObject *inputFBO);

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
	std::string log;

	vertexGeometry renderPlane;
	framebufferObject B;

	float iterationCounter;

	GLSLProgram fxaaShaderPrg;
	GLSLProgram poissonShaderPrg;
	GLSLProgram inpaintingShaderPrg;
	GLSLProgram idleShaderPrg;
	GLSLProgram stampShaderPrg;
	GLSLProgram distanceShaderPrg;
	GLSLProgram maskCreationShaderPrg;
};

#endif
