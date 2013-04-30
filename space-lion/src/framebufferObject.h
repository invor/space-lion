#ifndef framebufferObject_h
#define framebufferObject_h

#include "GL/glew.h"
#include <vector>
#include <iostream>

class framebufferObject
{
private:
	GLuint handle;
	std::vector<GLuint> colorbuffer;
	GLuint depthbuffer;
	GLuint stencilbuffer;

	int width;
	int height;

public:
	framebufferObject();
	~framebufferObject();

	framebufferObject(int w, int h, bool hasDepth, bool hasStencil);

	bool createColorAttachment(GLint index, GLenum internalFormat, GLenum format, GLenum type);

	void bind();
	void bindColorbuffer(int index);
	void bindDepthbuffer();
	void bindStencilbuffer();

	bool checkStatus();

	int getWidth() {return width;}
	int getHeight() {return height;}
};

#endif