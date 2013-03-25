#include "poissonImageProcessor.h"

bool poissonImageProcessor::initShaderProgram()
{
	if(!shaderPrg.initShaders(POISSON)) return false;
	return true;
}

//void poissonImageProcessor::render(framebufferObject inputFbo)
void poissonImageProcessor::render(GLuint inputImage)
{
	shaderPrg.use();
	renderPlane.draw(GL_TRIANGLES,6,0);
}