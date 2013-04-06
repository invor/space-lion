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

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	shaderPrg.setUniform("inputImage",0);
	glBindTexture(GL_TEXTURE_2D, inputImage);

	renderPlane.draw(GL_TRIANGLES,6,0);
}

void poissonImageProcessor::render(framebufferObject *currentFrame, framebufferObject *previousFrame, int iterations)
{
	shaderPrg.use();
	for(int i=0; i<iterations; i++)
	{
		//B.bind();
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		shaderPrg.setUniform("inputImage",0);
		currentFrame->bindColorbuffer();

		renderPlane.draw(GL_TRIANGLES,6,0);

		//currentFrame.bind();
		//glEnable(GL_TEXTURE_2D);
		//glActiveTexture(GL_TEXTURE0);
		//shaderPrg.setUniform("inputImage",0);
		//B.bindColorbuffer();
		//
		//renderPlane.draw(GL_TRIANGLES,6,0);
	}
}