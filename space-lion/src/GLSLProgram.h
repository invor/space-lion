#ifndef GLSLProgram_h
#define GLSLProgram_h

#include "GL\glew.h"
//openGL Math Lib
#include <glm/glm.hpp>
#include <glm/core/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>

//pragmas seem to be only necessary in windows
#ifdef _WIN32
	#pragma comment(lib,"glew32.lib")
#endif

enum shaderType {PHONG,FLAT};

class GLSLProgram
{
private:
	shaderType type;
	GLuint handle;
	bool linkStatus;
	std::string shaderlog;

	char* readShaderFile(const char *path);
	GLuint getUniformLocation(const char *name);
public:
	GLSLProgram();
	~GLSLProgram();

	GLSLProgram(shaderType);

	int getType() {return type;}

	bool compileShaderFromFile(const char *path, GLenum shaderType);
	bool link();
	bool use();
	std::string getLog();
	GLuint getHandle();
	bool isLinked();
	void bindAttribLocation(GLuint location, const char *name);
	void bindFragDataLocation(GLuint location, const char *name);
	void setUniform(const char *name, const glm::vec3 &v);
	void setUniform(const char *name, const glm::vec4 &v);
	void setUniform(const char *name, const glm::mat4 &m);
	void setUniform(const char *name, const glm::mat3 &m);
	void setUniform(const char *name, int i);
	void setUniform(const char *name, float f);
	void setUniform(const char *name, bool b);
	void printActiveUniforms();
	void printActiveAttributes();
};

#endif GLSLProgram
