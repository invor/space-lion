#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class postProcessor
{
public:
	postProcessor() : B(0,0,false,false,false) {}
	~postProcessor() {}

	postProcessor(int w, int h) : B(w,h,true,true,false) {}

	bool init();

	const std::string& getLog() {return log;}

	void applyFxaa(GLuint inputImage);
	void applyFxaa(framebufferObject *currentFrame);

	void imageToFBO(GLuint inputImage);
	void FBOToFBO(framebufferObject *inputFBO);

	void applyPoisson(GLuint inputImage);
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, glm::vec2 lowerBound, glm::vec2 upperBound);
	void applyPoisson(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations, GLuint);

private:
	std::string log;

	vertexGeometry renderPlane;
	framebufferObject B;

	GLSLProgram fxaaShaderPrg;
	GLSLProgram poissonShaderPrg;
	GLSLProgram idleShaderPrg;
};

#endif
