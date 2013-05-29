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

	/*	Applies a 3x3 seperated gaussian the first color attachment of the input framebuffer*/
	void applyGaussian(framebufferObject *currentFrame);

	/*
	/	Render the input texture to the currently bound framebuffer.
	*/
	void imageToFBO(GLuint inputImage);

	/*
	/	Render the color attachment of the input framebuffer to the currently bound framebuffer.
	*/
	void FBOToFBO(framebufferObject *inputFBO);

protected:
	std::string log;

	vertexGeometry renderPlane;

	framebufferObject B;

	GLSLProgram fxaaShaderPrg;
	GLSLProgram idleShaderPrg;
	GLSLProgram gaussianShaderPrg;
};

#endif
