#include "vertexGeometry.h"


vertexGeometry::vertexGeometry() : filename("0")
{
}


vertexGeometry::~vertexGeometry()
{
}

vertexGeometry::vertexGeometry(const char *fn) : filename(fn)
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
		//glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glGenBuffers(1, &iboHandle);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	}
	else
	{
		glBindVertexArray(vaHandle);
		//glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexArray), indexArray, GL_STATIC_DRAW);
	glBindBuffer(0,0);
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

void vertexGeometry::bindIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
}

void vertexGeometry::draw(GLenum type, GLint count, int indexOffset)
{
	glBindVertexArray(vaHandle);
	glDrawElements(type, count, GL_UNSIGNED_BYTE, (GLvoid*)(sizeof(GLubyte) * indexOffset));
}

void vertexGeometry::setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	glBindVertexArray(0);
}

void vertexGeometry::setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
}

void vertexGeometry::setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribLPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
}