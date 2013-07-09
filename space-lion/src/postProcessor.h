#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"
#include "resourceManager.h"

class postProcessor
{
public:
	postProcessor() : B(0,0,false,false), gaussianBackBuffer(0,0,false,false) {}
	~postProcessor() {}

	postProcessor(int w, int h) : B(w,h,false,false), gaussianBackBuffer(w,h,false,false) {}

	bool init(resourceManager* resourceMngr);

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(framebufferObject *currentFrame);

	/*
	/	Applies a seperated gaussian the first color attachment of the input framebuffer
	*/
	void applyGaussian(framebufferObject *inputFbo, framebufferObject *targetFbo, float sigma, int stencilRadius);

	/*
	/	Computes the gradient vector
	*/
	void computeGradient(framebufferObject *inputFbo, framebufferObject *targetFbo);

	/*
	/	Computes the hesse matrix for a given gradient field	
	*/
	void computeHesse(framebufferObject *inputFbo, framebufferObject *targetFbo);

	/*
	/	Render the input texture to the currently bound framebuffer.
	*/
	void imageToFBO(GLuint inputImage);

	/*
	/	Render the color attachment of the input framebuffer to the currently bound framebuffer.
	*/
	void FBOToFBO(framebufferObject *inputFbo);

protected:
	std::string log;

	vertexGeometry renderPlane;

	framebufferObject B;
	/*	FBO used for the seperated gaussian. Not to be used outside the gaussian method! */
	framebufferObject gaussianBackBuffer;

	GLSLProgram *fxaaShaderPrg;
	GLSLProgram *idleShaderPrg;
	GLSLProgram *gaussianShaderPrg;
	GLSLProgram *gradientShaderPrg;
	GLSLProgram *hesseShaderPrg;
};

#endif
