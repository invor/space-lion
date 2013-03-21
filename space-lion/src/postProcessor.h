#ifndef postProcessor_h
#define postProcessor_h

#include "vertexGeometry.h"
#include "GLSLProgram.h"
#include "framebufferObject.h"

class postProcessor
{
public:
	postProcessor();
	~postProcessor();

	void render(framebufferObject inputFbo);

private:
	const GLSLProgram shaderProgram;
	const vertexGeometry renderPlane;
};

#endif