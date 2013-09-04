#include "framebufferObject.h"

FramebufferObject::FramebufferObject()
{
}

FramebufferObject::~FramebufferObject()
{
}

FramebufferObject::FramebufferObject(int w, int h, bool hasDepth, bool hasStencil) : width(w), height(h)
{
	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

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

bool FramebufferObject::createColorAttachment(GLenum internalFormat, GLenum format, GLenum type)
{	
	GLint maxAttachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);

	if(colorbuffer.size() == (GLuint) maxAttachments) 
	{
		std::cout<<"Maximum amount of color attachments reached.\n";
		return false;
	}

	int bufsSize = colorbuffer.size();
	colorbuffer.push_back(GLuint());
	std::vector<GLuint>::iterator lastElement = (--(colorbuffer.end()));

	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	
	glGenTextures(1, &*lastElement);
	glActiveTexture(GL_TEXTURE0+bufsSize);
	glBindTexture(GL_TEXTURE_2D, *lastElement);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+bufsSize, GL_TEXTURE_2D, *lastElement, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void FramebufferObject::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout<<"Tried to use incomplete FBO. Fallback to default FBO.\n";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		int bufsSize = colorbuffer.size();
		GLenum* drawBufs = new GLenum[bufsSize];
		for(GLint i=0; i < bufsSize; i++)
		{
			drawBufs[i] = (GL_COLOR_ATTACHMENT0+i);
		}
		glDrawBuffers(bufsSize, drawBufs);
	}
}

void FramebufferObject::bindColorbuffer(int index)
{
	//std::vector<GLuint>::iterator itr = colorbuffer.begin();
	//for(int i = 0; i < index; i++) ++itr;

	//glBindTexture(GL_TEXTURE_2D, *itr);
	glBindTexture(GL_TEXTURE_2D, colorbuffer[index]);
}

void FramebufferObject::bindDepthbuffer()
{
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
}

void FramebufferObject::bindStencilbuffer()
{
	glBindTexture(GL_TEXTURE_2D, stencilbuffer);
}

bool FramebufferObject::checkStatus()
{
	if(glCheckFramebufferStatus(handle) == GL_FRAMEBUFFER_COMPLETE) return true;
	return false;
}
