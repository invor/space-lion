#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class postProcessor
{
public:
	postProcessor() : B(0,0,false,false,false), iterationCounter(0) {}
	~postProcessor() {}

	postProcessor(int w, int h) : B(w,h,true,true,false), iterationCounter(0) {}

	bool init();

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(framebufferObject *currentFrame);

	/*
	/	Render the input texture to the currently bound framebuffer using a mask.
	/	Pixels covered by black areas of the mask are rendered black.
	*/
	void applyMaskToImageToFBO(GLuint inputImage, GLuint mask, int w, int h);
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
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound);
	/*
	/	Apply poisson image editing to [a] region[s] given by an image-mask.
	*/
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, GLuint mask);

private:
	std::string log;

	vertexGeometry renderPlane;
	framebufferObject B;

	float iterationCounter;

	GLSLProgram fxaaShaderPrg;
	GLSLProgram poissonShaderPrg;
	GLSLProgram idleShaderPrg;
	GLSLProgram stampShaderPrg;
};

#endif
