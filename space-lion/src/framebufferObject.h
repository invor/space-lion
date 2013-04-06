#ifndef framebufferObject_h
#define framebufferObject_h

#include "GL/glew.h"
//#include <vector>
#include <iostream>

class framebufferObject
{
private:
	GLuint handle;
	//	A single color attachment should be enough for now
	//std::vector<GLuint> colorbuffer;
	GLuint colorbuffer;
	GLuint depthbuffer;
	GLuint stencilbuffer;

	int width;
	int height;

public:
	framebufferObject();
	~framebufferObject();

	framebufferObject(int w, int h, bool hasColor, bool hasDepth, bool hasStencil);

	void bind();
	void bindColorbuffer();
	void bindDepthbuffer();
	void bindStencilbuffer();

	bool checkStatus();

	int getWidth() {return width;}
	int getHeight() {return height;}
};

#endif