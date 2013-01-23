#ifndef vertexBufferObject_h
#define vertexBufferObject_h

#include "GL\glew.h"
#include "vertexStructs.h"

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class vertexBufferObject
{
private:
	const int id;

	GLuint handle;

public:
	vertexBufferObject(void);
	~vertexBufferObject(void);

	vertexBufferObject(int);

	bool bufferDataFromArray(const vertex3 *vertexArray);
	bool bufferDataFromFile(const char *path);

	void bindBuffer();
	void bindVertexArray();
	void draw(GLenum type, GLint count);

	bool setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
};

#endif