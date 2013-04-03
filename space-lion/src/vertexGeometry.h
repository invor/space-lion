#ifndef vertexGeometry_h
#define vertexGeometry_h

#include "GL/glew.h"
#include "vertexStructs.h"

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class vertexGeometry
{
private:
	const char *filename;

	//vertex array
	GLuint vaHandle;
	//vertex buffer object
	GLuint vboHandle;
	//index buffer object
	GLuint iboHandle;

public:
	vertexGeometry(void);
	~vertexGeometry(void);

	vertexGeometry(const char*);

	bool bufferDataFromArray(const vertex3 *vertexArray, const GLubyte *indexArray, const GLsizei vaSize, const GLsizei viSize);
	bool bufferDataFromFile(const char *path);

	void bindVertexBuffer();
	void bindVertexArray();
	void bindIndexBuffer();
	void draw(GLenum type, GLint count, int indexOffset);

	void setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

	const char* getFilename() {return filename;}
};

#endif
