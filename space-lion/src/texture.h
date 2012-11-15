//#pragma once
#include "GL\glew.h"
#include "SOIL.h"
#include <iostream>

class texture
{
private:
	int id;

	GLuint handle;
public:
	texture(void);
	~texture(void);

	bool loadFromFile(const char *path);
	virtual void texParameteri(GLenum, GLenum) = 0;
};

