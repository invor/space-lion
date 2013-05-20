#ifndef texture3D_h
#define texture3D_h

#include "texture.h"

class texture3D : public texture
{
private:

public:
	void bindTexture() const;
	void texParameteri(GLenum, GLenum);

	/*
	/	load a texture from a local file
	*/
	bool loadTextureFile(std::string inputPat, int dimX, int dimY, int dimZ);

	/*
	/	load a texture from an array of floats
	*/
	bool loadArrayF(int dimX, int dimY, int dimZ, float* data);
};

#endif
