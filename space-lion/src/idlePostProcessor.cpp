#include "idlePostProcessor.h"

bool idlePostProcessor::initShaderProgram()
{
	if(!shaderPrg.initShaders(IDLE)) return false;
	return true;
}

void idlePostProcessor::render(GLuint inputImage)
{
	shaderPrg.use();

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void idlePostProcessor::render(framebufferObject *inputFBO)
{
	shaderPrg.use();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderPrg.setUniform("inputImage",0);
	inputFBO->bindColorbuffer();

	renderPlane.draw(GL_TRIANGLES,6,0);
}