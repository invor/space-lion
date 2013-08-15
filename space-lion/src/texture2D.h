#ifndef texture2D_h
#define texture2D_h

#include "texture.h"

class Texture2D : public Texture
{
private:

public:
	void bindTexture() const;
	void texParameteri(GLenum, GLenum);

	/*
	/	load a texture from a local file
	*/
	bool loadTextureFile(std::string inputPath);

	/*
	/	load a texture from an array of floats
	*/
	bool loadArrayF(int dimX, int dimY, float* data);
};

#endif
