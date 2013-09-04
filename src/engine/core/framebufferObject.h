#ifndef framebufferObject_h
#define framebufferObject_h

#include <GL/glew.h>
#include <vector>
#include <iostream>

class FramebufferObject
{
private:
	GLuint handle;
	std::vector<GLuint> colorbuffer;
	GLuint depthbuffer;
	GLuint stencilbuffer;

	int width;
	int height;

public:
	FramebufferObject();
	~FramebufferObject();

	FramebufferObject(int w, int h, bool hasDepth, bool hasStencil);

	/*
	/	Adds one color attachment to the framebuffer.
	/	
	/	New colorbuffers are added at the end of the colorbuffer vector.
	/	The index i within the storage vector and the used attachment point
	/	GL_COLOR_ATTACHMENTi will be the same.
	*/
	bool createColorAttachment(GLenum internalFormat, GLenum format, GLenum type);

	void bind();
	void bindColorbuffer(int index);
	void bindDepthbuffer();
	void bindStencilbuffer();

	bool checkStatus();

	int getWidth() {return width;}
	int getHeight() {return height;}
};

#endif