#ifndef texture2D_h
#define texture2D_h

#include "texture.h"

class Texture2D : public Texture
{
private:

public:
	Texture2D() {}
	Texture2D(std::string in_filename) : Texture(in_filename) {}

	void bindTexture() const;
	void texParameteri(GLenum, GLenum);

	/*	Load a texture from an array of floats */
	bool loadArrayF(int dimX, int dimY, float* data);

	/*	Load a texture from an array of characters (bytes) */
	bool loadArrayC(int dimX, int dimY, char* data);
};

#endif
