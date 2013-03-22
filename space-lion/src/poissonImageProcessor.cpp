#include "poissonImageProcessor.h"

bool poissonImageProcessor::initShaderProgram()
{
	shaderPrg = GLSLProgram(POISSON);
	if(!shaderPrg.compileShaderFromFile("../resources/shaders/v_poisson.glsl",GL_VERTEX_SHADER)) return false;
	if(!shaderPrg.compileShaderFromFile("../resources/shaders/f_poisson.glsl",GL_FRAGMENT_SHADER)) return false;
	shaderPrg.bindAttribLocation(0,"vPosition");
	shaderPrg.bindAttribLocation(1,"vUVCoord");
	if(!shaderPrg.link()) return false;
	std::cout<<shaderPrg.getLog();
	glUseProgram(0);
	return true;
}

//void poissonImageProcessor::render(framebufferObject inputFbo)
void poissonImageProcessor::render()
{
	shaderPrg.use();
	renderPlane.draw(GL_TRIANGLES,2,0);
}