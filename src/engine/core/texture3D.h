#ifndef texture3D_h
#define texture3D_h

#define _CRT_SECURE_NO_DEPRECATE

#include "texture.h"
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>

class Texture3D : public Texture
{
public:
	Texture3D() {}
	Texture3D(std::string in_filename) : Texture(in_filename) {}

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

private:
	Texture3D(Texture3D &cpy) {}
};

#endif
