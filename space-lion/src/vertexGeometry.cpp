#include "vertexGeometry.h"


vertexGeometry::vertexGeometry(void) : id()
{
}


vertexGeometry::~vertexGeometry(void)
{
}

vertexGeometry::vertexGeometry(int in_id) : id(in_id)
{
}

bool vertexGeometry::bufferDataFromFile(const char *path)
{
	return false;
}

bool vertexGeometry::bufferDataFromArray(const vertex3 *vertexArray, const GLubyte *indexArray)
{
	if(vaHandle == NULL || vboHandle == NULL)
	{
		glGenVertexArrays(1, &vaHandle);
		glBindVertexArray(vaHandle);
		glGenBuffers(1, &vboHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glGenBuffers(1, &iboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	}
	else
	{
		glBindVertexArray(vaHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexArray), indexArray, GL_STATIC_DRAW);
	glBindVertexArray(0);

	return true;
}

void vertexGeometry::bindVertexBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
}

void vertexGeometry::bindVertexArray()
{
	glBindVertexArray(vaHandle);
}

void vertexGeometry::draw(GLenum type, GLint count)
{
	glBindVertexArray(vaHandle);
	glDrawArrays(type, 0, count);
}

bool vertexGeometry::setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	glBindVertexArray(0);
	return true;
}

bool vertexGeometry::setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	return true;
}

bool vertexGeometry::setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribLPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	return true;
}