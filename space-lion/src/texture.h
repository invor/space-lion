#ifndef texture_h
#define texture_h

#include "GL\glew.h"
#include <iostream>

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class texture
{
private:
	int id;

	GLuint handle;
public:
	texture();
	texture(int inputId);
	~texture();

	//	load a texture from a local file
	bool load(const char *path);
	//	load a texture from an array of floats
	bool load(int dimX, int dimY, float* data);
	void texParameteri(GLenum, GLenum);
};

#endif