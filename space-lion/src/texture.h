#ifndef texture_h
#define texture_h

#include "GL/glew.h"
#include "GL/glfw.h"
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

	/*
	/	Specifies which texture type (1D, 2D or 3D) is used.
	*/
	const GLenum type;
public:
	texture();
	texture(const char*, GLenum);
	~texture();

	void bindTexture() const;

	//	load a texture from a local file
	bool loadTexture2D(const char *path);
	//	load a texture from an array of floats
	bool loadTexture2D(int dimX, int dimY, float* data);
	bool loadTexture3D(int dimX, int dimY, int dimZ, const char* path);
	void texParameteri(GLenum, GLenum);

	const char* getFilename() {return filename;}
};

#endif
