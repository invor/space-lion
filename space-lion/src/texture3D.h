#ifndef texture3D_h
#define texture3D_h

#include "texture.h"
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>

class texture3D : public texture
{
private:

public:
	void bindTexture() const;
	void texParameteri(GLenum, GLenum);

	/*
	/	load a texture from a local file
	*/
	bool loadTextureFile(std::string inputPat, glm::ivec3 resolution);

	/*
	/	load a texture from an array of floats
	*/
	bool loadArrayF(int dimX, int dimY, int dimZ, float* data);
};

#endif
