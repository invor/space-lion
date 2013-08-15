#ifndef postProcessor_h
#define postProcessor_h

#include "mesh.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"
#include "resourceManager.h"

class PostProcessor
{
public:
	PostProcessor() : B(0,0,false,false), gaussianBackBuffer(0,0,false,false) {}
	~PostProcessor() {}

	PostProcessor(int w, int h) : B(w,h,false,false), gaussianBackBuffer(w,h,false,false) {}

	bool init(ResourceManager* resourceMngr);

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(FramebufferObject *currentFrame);

	/*
	/	Applies a seperated gaussian the first color attachment of the input framebuffer
	*/
	void applyGaussian(FramebufferObject *inputFbo, FramebufferObject *targetFbo, float sigma, int stencilRadius);

	/*
	/	Computes the gradient vector
	*/
	void computeGradient(FramebufferObject *inputFbo, FramebufferObject *targetFbo);

	/*
	/	Computes the hesse matrix for a given gradient field	
	*/
	void computeHesse(FramebufferObject *inputFbo, FramebufferObject *targetFbo);

	/*
	/	Computes the structure tensor (matrix) for a given scalar field (image)	
	*/
	void computeStructureTensor(FramebufferObject *inputFbo, FramebufferObject *targetFbo);

	/*
	/	Render the input texture to the currently bound framebuffer.
	*/
	void imageToFBO(GLuint inputImage);

	/*
	/	Render the color attachment of the input framebuffer to the currently bound framebuffer.
	*/
	void FBOToFBO(FramebufferObject *inputFbo);

protected:
	std::string log;

	Mesh renderPlane;

	FramebufferObject B;
	/*	FBO used for the seperated gaussian. Not to be used outside the gaussian method! */
	FramebufferObject gaussianBackBuffer;

	GLSLProgram *fxaaShaderPrg;
	GLSLProgram *idleShaderPrg;
	GLSLProgram *gaussianShaderPrg;
	GLSLProgram *gradientShaderPrg;
	GLSLProgram *hesseShaderPrg;
	GLSLProgram *structureTensorShaderPrg;
};

#endif
