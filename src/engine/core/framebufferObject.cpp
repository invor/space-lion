#include "framebufferObject.h"

FramebufferObject::FramebufferObject() : m_handle(0), m_depthbuffer(0), m_stencilbuffer(0), m_width(0), m_height(0)
{
}

FramebufferObject::~FramebufferObject()
{
	/*	Delete framebuffer resources. Texture delete themselves when the vector is destroyed. */	
	if (m_depthbuffer != 0) glDeleteRenderbuffers(1, &m_depthbuffer);

	/*	Delete framebuffer object */
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
	glDeleteFramebuffers(1, &m_handle);
}

FramebufferObject::FramebufferObject(int width, int height, bool has_depth, bool has_stencil) : m_width(width), m_height(height)
{
	glGenFramebuffers(1, &m_handle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);

	if(has_depth)
	{
		glGenRenderbuffers(1, &m_depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	else
	{
		m_depthbuffer = 0;
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

	if(m_colorbuffers.size() == (GLuint) maxAttachments) 
	{
		std::cout<<"Maximum amount of color attachments reached.\n";
		return false;
	}

	unsigned int bufsSize = m_colorbuffers.size();
	m_colorbuffers.push_back(Texture2D(""));
	std::vector<Texture2D>::iterator lastElement = (--(m_colorbuffers.end()));

	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
	
	lastElement->load(internalFormat, m_width, m_height, format, type, NULL);
	lastElement->bindTexture();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+bufsSize, GL_TEXTURE_2D, lastElement->getHandle(), 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void FramebufferObject::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout<<"Tried to use incomplete FBO. Fallback to default FBO.\n";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		unsigned int bufsSize = m_colorbuffers.size();
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
	if (index < m_colorbuffers.size()) m_colorbuffers[index].bindTexture();
}

void FramebufferObject::bindDepthbuffer()
{
	glBindTexture(GL_TEXTURE_2D, m_depthbuffer);
}

void FramebufferObject::bindStencilbuffer()
{
	glBindTexture(GL_TEXTURE_2D, m_stencilbuffer);
}

bool FramebufferObject::checkStatus()
{
	if(glCheckFramebufferStatus(m_handle) == GL_FRAMEBUFFER_COMPLETE) return true;
	return false;
}

void FramebufferObject::resize(int new_width, int new_height)
{
	for (std::vector<Texture2D>::iterator itr = m_colorbuffers.begin(); itr != m_colorbuffers.end(); ++itr)
	{
		itr->reload(new_width, new_height, NULL);
	}
}