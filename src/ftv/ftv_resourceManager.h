/**
* \class Ftv_ResourceManager
* 
* \brief Loads and manages ftv related graphic resources
* 
* 
* \author Michael Becher
* 
* \date 23rd September 2013
*/

#ifndef ftv_resourceManager_h
#define ftv_resourceManager_h

/*	Include space-lion headers */
#include "../engine/core/resourceManager.h"

#define DEBUG_OUTPUT 0

enum ftv_shaderType	{ DISTANCEMAPPING, COHERENCE, STAMP, FTV_POISSON, FTV_INPAINTING, FTV_IMPROVED_INPAINTING, FTV_MASK, FTV_VOLUME_RAYCASTING, FTV_GAUSSIAN, FTV_MASK_SHRINK, FTV_TEXTURE_ADVECTION };

class Ftv_ResourceManager : public ResourceManager
{
public:
	Ftv_ResourceManager() {}
	~Ftv_ResourceManager() {}

	/**
	 * Creates a GLSLprogram object
	 * \param type Specifies which shader program should be created
	 * \param inOutPrgPtr A pointer set to the newly created shader program via in-out parameter
	 * \retrun Returns true if GLSLprogram was succesfully created, false otherwise
	 */
	bool createFtvShaderProgram(ftv_shaderType type, std::shared_ptr<GLSLProgram> &inOutPrgPtr);

private:
	
	/** List containing all shader programs */
	std::list<std::shared_ptr<GLSLProgram>> ftv_shader_program_list;
};

#endif