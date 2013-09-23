#include "ftv_resourceManager.h"


bool Ftv_ResourceManager::createFtvShaderProgram(ftv_shaderType type, std::shared_ptr<GLSLProgram> &inOutPrgPtr)
{
	/*	Check list of shader programs for the shader type */
	for(std::list<std::shared_ptr<GLSLProgram>>::iterator i = ftv_shader_program_list.begin(); i != ftv_shader_program_list.end(); ++i)
	{
		if(((*i)->getType())==type){
			inOutPrgPtr = (*i);
			return true;
		}
	}

	std::shared_ptr<GLSLProgram> shaderPrg(new GLSLProgram());
	shaderPrg->init();
	std::string vertSource;
	std::string fragSource;

	switch (type)
	{
		case FTV_POISSON: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_poisson.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case STAMP: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_stamp.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case FTV_INPAINTING: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_imageInpainting.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case DISTANCEMAPPING: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_distanceMapping.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case FTV_MASK: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_mask.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			shaderPrg->bindFragDataLocation(0, "inpaintingMask");
			shaderPrg->bindFragDataLocation(1, "distanceMap");
			break; }
		case FTV_VOLUME_RAYCASTING : {
			vertSource = readShaderFile("../resources/shaders/v_ftv_volRen.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_volRen.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(3, "vColour");
			break; }
		case COHERENCE: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_coherence.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case FTV_IMPROVED_INPAINTING: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_improvedInpainting.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case FTV_GAUSSIAN: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_seperatedGaussian.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		case FTV_MASK_SHRINK: {
			vertSource = readShaderFile("../resources/shaders/v_genericPostProc.glsl");
			fragSource = readShaderFile("../resources/shaders/f_ftv_shrinkMask.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			shaderPrg->bindFragDataLocation(0, "inpaintingMask");
			shaderPrg->bindFragDataLocation(1, "distanceMap");
			break; }
		case FTV_TEXTURE_ADVECTION: {
			vertSource = readShaderFile("../resources/shaders/v_ftv_textureAdvection.glsl");
			fragSource = readShaderFile("../resources/shaders/f_idle.glsl");
			shaderPrg->bindAttribLocation(0, "vPosition");
			shaderPrg->bindAttribLocation(1, "vUVCoord");
			break; }
		default: {
			return false;
			break; }
	}


	if(!shaderPrg->compileShaderFromString(&vertSource,GL_VERTEX_SHADER)){ std::cout<<shaderPrg->getLog(); return false;}
	if(!shaderPrg->compileShaderFromString(&fragSource,GL_FRAGMENT_SHADER)){ std::cout<<shaderPrg->getLog(); return false;}
	if(!shaderPrg->link()){ std::cout<<shaderPrg->getLog(); return false;}

	inOutPrgPtr = shaderPrg;
	shader_program_list.push_back(std::move(shaderPrg));

	return true;
}