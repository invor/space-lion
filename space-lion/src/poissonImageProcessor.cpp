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
	glEnable(GL_TEXTURE_2D);
	for(int i=0; i<iterations; i++)
	{
		B.bind();
		glViewport(0,0,B.getWidth(),B.getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		shaderPrg.setUniform("lowerBound", glm::vec2(0.0f,0.0f));
		shaderPrg.setUniform("upperBound", glm::vec2(1.0f,1.0f));
		glActiveTexture(GL_TEXTURE0);
		shaderPrg.setUniform("inputImage",0);
		currentFrame->bindColorbuffer();

		renderPlane.draw(GL_TRIANGLES,6,0);

		currentFrame->bind();
		glViewport(0,0,currentFrame->getWidth(),currentFrame->getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderPrg.setUniform("imgDim", glm::vec2(B.getWidth(), B.getHeight()));
		shaderPrg.setUniform("lowerBound", glm::vec2(0.0f,0.0f));
		shaderPrg.setUniform("upperBound", glm::vec2(1.0f,1.0f));
		glActiveTexture(GL_TEXTURE0);
		shaderPrg.setUniform("inputImage",0);
		B.bindColorbuffer();
		
		renderPlane.draw(GL_TRIANGLES,6,0);
	}
}