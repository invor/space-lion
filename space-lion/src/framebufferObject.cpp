#include "framebufferObject.h"

framebufferObject::framebufferObject()
{
}

framebufferObject::~framebufferObject()
{
}

framebufferObject::framebufferObject(int w, int h, bool hasColor, bool hasDepth, bool hasStencil) : width(w), height(h)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	if(hasColor)
	{
		glGenTextures(1, &colorbuffer);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if(hasDepth)
	{
		glGenRenderbuffers(1, &depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	/*
	/	TODO: stencilbuffer
	*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void framebufferObject::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout<<"Tried to use incomplete FBO. Fallback to default FBO.\n";
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
	else
	{
		GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, drawBufs);
	}
}

void framebufferObject::bindColorbuffer()
{
	glBindTexture(GL_TEXTURE_2D, colorbuffer);
}

void framebufferObject::bindDepthbuffer()
{
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
}

void framebufferObject::bindStencilbuffer()
{
	glBindTexture(GL_TEXTURE_2D, stencilbuffer);
}

bool framebufferObject::checkStatus()
{
	if(glCheckFramebufferStatus(handle) == GL_FRAMEBUFFER_COMPLETE) return true;
	return false;
}