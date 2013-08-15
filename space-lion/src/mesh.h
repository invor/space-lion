#ifndef mesh_h
#define mesh_h

#include <string>
#include "GL/glew.h"
#include "vertexStructs.h"

//pragma seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

class Mesh
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
	Mesh();
	~Mesh();

	Mesh(const std::string fn);

	bool bufferDataFromArray(const Vertex_p *vertexArray, const GLuint *indexArray, const GLsizei vaSize, const GLsizei viSize);
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
