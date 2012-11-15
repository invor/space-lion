//#pragma once
#include "GL\glew.h"

#include "vertexStructs.h"

class vertexBufferObject
{
private:
	int id;

	GLuint handle;

public:
	vertexBufferObject(void);
	~vertexBufferObject(void);

	bool bufferDataFromArray(const vertex3 *vertexArray);
	bool bufferDataFromFile(const char *path);

	void bindBuffer();
	void bindVertexArray();
	void draw(GLenum type, GLint count);

	bool setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
};