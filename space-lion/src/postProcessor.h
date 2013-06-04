#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class postProcessor
{
public:
	postProcessor() : B(0,0,false,false) {}
	~postProcessor() {}

	postProcessor(int w, int h) : B(w,h,true,false) {}

	bool init();

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(framebufferObject *currentFrame);

	/*
	/	Applies a 3x3 seperated gaussian the first color attachment of the input framebuffer
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

	GLSLProgram fxaaShaderPrg;
	GLSLProgram idleShaderPrg;
	GLSLProgram gaussianShaderPrg;
	GLSLProgram gradientShaderPrg;
	GLSLProgram hesseShaderPrg;
};

#endif
