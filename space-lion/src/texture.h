#ifndef texture_h
#define texture_h

#include "GL/glew.h"
#include "glfw.h"
#include <iostream>

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class texture
{
private:
	//	Integer ids seemed like a bad idea for loading texture files, that themselves -unlike custom material files- won't contain that id.
	const char *filename;

	GLuint handle;
public:
	texture();
	texture(const char*);
	~texture();

	void bindTexture();

	//	load a texture from a local file
	bool load(const char *path);
	//	load a texture from an array of floats
	bool load(int dimX, int dimY, float* data);
	void texParameteri(GLenum, GLenum);

	const char* getFilename() {return filename;}
};

#endif