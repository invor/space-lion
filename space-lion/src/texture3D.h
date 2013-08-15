#ifndef texture3D_h
#define texture3D_h

#include "texture.h"
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>

class Texture3D : public Texture
{
private:

public:
	void bindTexture() const;
	void texParameteri(GLenum, GLenum);

	/*
	/	load a texture from a local file
	*/
	bool loadTextureFile(std::string inputPath, glm::ivec3 resolution);

	/*
	/	load a texture from an array of floats
	*/
	bool loadArrayF(float* data, glm::ivec3 resolution, GLenum internalFormat, GLenum format);
};

#endif
