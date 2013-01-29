#ifndef vertexGeometry_h
#define vertexGeometry_h

#include "GL\glew.h"
#include "vertexStructs.h"

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class vertexGeometry
{
private:
	const int id;

	//vertex array
	GLuint vaHandle;
	//vertex buffer object
	GLuint vboHandle;
	//index buffer object
	GLuint iboHandle;

public:
	vertexGeometry(void);
	~vertexGeometry(void);

	vertexGeometry(int);

	bool bufferDataFromArray(const vertex3 *vertexArray, const GLubyte *indexArray);
	bool bufferDataFromFile(const char *path);

	void bindVertexBuffer();
	void bindVertexArray();
	void draw(GLenum type, GLint count, int indexOffset);

	bool setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	bool setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

	const int getId() {return id;}
};

#endif