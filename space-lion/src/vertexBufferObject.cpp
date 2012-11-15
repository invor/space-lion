#include "vertexBufferObject.h"


vertexBufferObject::vertexBufferObject(void)
{
}


vertexBufferObject::~vertexBufferObject(void)
{
}

bool vertexBufferObject::bufferDataFromFile(const char *path)
{
	return false;
}

bool vertexBufferObject::bufferDataFromArray(const vertex3 *vertexArray)
{
	if(handle = NULL)
	{
		glGenVertexArrays(1, &handle);
		glBindVertexArray(handle);
		glGenBuffers(1, &handle);
		glBindBuffer(GL_ARRAY_BUFFER, handle);
	}
	else
	{
		glBindVertexArray(handle);
		glBindBuffer(GL_ARRAY_BUFFER, handle);
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glBindVertexArray(0);

	return true;
}

void vertexBufferObject::bindBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void vertexBufferObject::bindVertexArray()
{
	glBindVertexArray(handle);
}

void vertexBufferObject::draw(GLenum type, GLint count)
{
	glBindVertexArray(handle);
	glDrawArrays(type, 0, count);
}

bool vertexBufferObject::setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(handle);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	glBindVertexArray(0);
	return true;
}

bool vertexBufferObject::setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(handle);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	return true;
}

bool vertexBufferObject::setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(handle);
	glEnableVertexAttribArray(index);
	glVertexAttribLPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	return true;
}