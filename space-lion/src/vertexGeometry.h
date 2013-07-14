#ifndef vertexGeometry_h
#define vertexGeometry_h

#include <string>
#include "GL/glew.h"
#include "vertexStructs.h"

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class vertexGeometry
{
private:
	const std::string filename;
	GLuint vertexCount;

	//vertex array
	GLuint vaHandle;
	//vertex buffer object
	GLuint vboHandle;
	//index buffer object
	GLuint iboHandle;

public:
	vertexGeometry(void);
	~vertexGeometry(void);

	vertexGeometry(const std::string fn);

	bool bufferDataFromArray(const vertex_p *vertexArray, const GLuint *indexArray, const GLsizei vaSize, const GLsizei viSize);
	bool bufferDataFromFile(const char *path);

	void bindVertexBuffer();
	void bindVertexArray();
	void bindIndexBuffer();
	void draw(GLenum type, GLint count, int indexOffset);

	void setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

	const std::string getFilename() {return filename;}
};

#endif
