#ifndef abstractPostProcessor_h
#define abstractPostProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class abstractPostProcessor
{
public:
	abstractPostProcessor();
	~abstractPostProcessor();

	bool init();
	//virtual void render(framebufferObject inputFbo) = 0;
	virtual void render() = 0;

	std::string getLog() {return log;}

protected:
	std::string log;

	GLSLProgram shaderPrg;
	vertexGeometry renderPlane;

	bool initRenderPlane();
	virtual bool initShaderProgram() = 0;
};

#endif