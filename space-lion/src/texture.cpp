#include "texture.h"


texture::texture(void)
{
}

texture::texture(int inputId)
{
	id = inputId;
}


texture::~texture(void)
{
}


bool texture::load(const char* path)
{
	return true;
}

bool texture::load(int dimX, int dimY, float *data)
{
	return true;
}
