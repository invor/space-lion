#ifndef framebufferObject_h
#define framebufferObject_h

#include "GL/glew.h"
#include <vector>

class framebufferObject
{
private:
	GLuint handle;
	std::vector<GLuint> colorbuffer;
	GLuint depthbuffer;
	GLuint stencilbuffer;

public:
	framebufferObject();
	~framebufferObject();

	void bind();
};

#endif