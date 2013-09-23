#ifndef mesh_h
#define mesh_h

#include <string>
#include <GL/glew.h>
#include "vertexStructs.h"
#include <iostream>
#include "../fbx/fbx_opengl_geometry.hpp"

class Mesh
{
private:
	const std::string filename;
	GLuint num_vertices;
	GLenum type;

	/*	Handle of the vertex array on the GPU */
	GLuint va_handle;
	/*	Handle of the vertex buffer object on the GPU */
	GLuint vbo_handle;
	/*	Handle of the index buffer object  on the GPU */
	GLuint ibo_handle;

public:
	Mesh();
	~Mesh();

	Mesh(const std::string fn);

	bool bufferDataFromArray(const Vertex_p *vertex_data, const GLuint *index_data, const GLsizei va_size, const GLsizei vi_size, GLenum mesh_type);

	void draw(int num_instances = 1);

	void setVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	void setVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

	const std::string getFilename() {return filename;}

	static std::shared_ptr<Mesh> loadFromFBX(const std::string &filename, FBX::OpenGL::BindAttribLocations locations = FBX::OpenGL::BindAttribLocations());
};

#endif
