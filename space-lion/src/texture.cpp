#include "texture.h"


texture::texture(void)
{
}

texture::texture(const char *fn) : filename(fn)
{
}


texture::~texture(void)
{
}

void texture::bindTexture()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

bool texture::load(const char* path)
{
	return true;
}

bool texture::load(int dimX, int dimY, float *data)
{
	//TODO: Add some checks

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,dimX,dimY,0,GL_RGB,GL_FLOAT,data);
	glBindTexture(GL_TEXTURE_2D,0);

	return true;
}

void texture::texParameteri(GLenum param_1, GLenum param_2)
{
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, param_1, param_2);
	glBindTexture(GL_TEXTURE_2D,0);
}