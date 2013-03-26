#include "fxaaPostProcessor.h"

bool fxaaPostProcessor::initShaderProgram()
{
	if(!shaderPrg.initShaders(FXAA)) return false;
	return true;
}

//void poissonImageProcessor::render(framebufferObject inputFbo)
void fxaaPostProcessor::render(GLuint inputImage)
{
	shaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw(GL_TRIANGLES,6,0);
}