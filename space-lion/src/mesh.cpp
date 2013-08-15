#include "mesh.h"


Mesh::Mesh() : filename("0"), vaHandle(0), vboHandle(0), iboHandle(0)
{
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &vboHandle);
	glDeleteBuffers(1, &iboHandle);
	glDeleteVertexArrays(1, &vaHandle);
}

Mesh::Mesh(const std::string fn) : filename(fn), vaHandle(0), vboHandle(0), iboHandle(0)
{
}

bool Mesh::bufferDataFromFile(const char *path)
{
	return false;
}

bool Mesh::bufferDataFromArray(const Vertex_p *vertexArray, const GLuint *indexArray, const GLsizei vaSize, const GLsizei viSize)
{
	if(vaHandle == 0 || vboHandle == 0 || iboHandle == 0)
	{
		glGenVertexArrays(1, &vaHandle);
		glGenBuffers(1, &vboHandle);
		glGenBuffers(1, &iboHandle);
	}

	glBindVertexArray(vaHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glBufferData(GL_ARRAY_BUFFER, vaSize, vertexArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, viSize, indexArray, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	vertexCount = (viSize/sizeof(GLuint));

	return true;
}

void Mesh::bindVertexBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
}

void Mesh::bindVertexArray()
{
	glBindVertexArray(vaHandle);
}

void Mesh::bindIndexBuffer()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
}

void Mesh::draw(GLenum type, GLint count, int indexOffset)
{
	glBindVertexArray(vaHandle);
	/*	If stored correctly in the VAO, there is no need to rebind buffers again */
	//glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	//glDrawElements(type, count, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLubyte) * indexOffset));
	glDrawElements(type, vertexCount, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * indexOffset));
}

void Mesh::setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	glBindVertexArray(vaHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	glEnableVertexAttribArray(index);
	glVertexAttribLPointer(index, size, type, stride, pointer);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
