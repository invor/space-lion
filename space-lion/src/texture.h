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
	texture(void);
	~texture(void);

	bool loadFromFile(const char *path);
	//virtual void texParameteri(GLenum, GLenum) = 0;
};

#endif