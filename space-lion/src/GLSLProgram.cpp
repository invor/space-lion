#include "GLSLProgram.h"

GLSLProgram::GLSLProgram()
{
	handle = glCreateProgram();
	linkStatus = false;
}

GLSLProgram::~GLSLProgram()
{
}


GLuint GLSLProgram::getUniformLocation(const char *name)
{
	return glGetUniformLocation(handle, name);
}

bool GLSLProgram::compileShaderFromString(const std::string * const source, GLenum shaderType)
{
	/* Check if the source is empty */
	if (source->empty())
	{
		shaderlog = "No shader source.";
		return false;
	}

	/* Create shader object */
	const GLchar* c_source = source->c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &c_source, NULL);

	/* Compile shader */
	glCompileShader(shader);

	/* Check for errors */
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE)
	{
		GLint logLen = 0;
		shaderlog = "";
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = new char[logLen];
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			shaderlog = log;
			delete [] log;
		}

		glDeleteShader(shader);
		return false;
	}

	//	GLint logLen = 0;
	//	shaderlog = "";
	//	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
	//	if(logLen > 0)
	//	{
	//		char *log = new char[logLen];
	//		GLsizei written;
	//		glGetShaderInfoLog(shader, logLen, &written, log);
	//		shaderlog = log;
	//		delete [] log;
	//	}

	/* Attach shader to program */
	glAttachShader(handle, shader);

	return true;
}

bool GLSLProgram::link()
{
	if( linkStatus ) return true;
	glLinkProgram(handle);

	GLint status = GL_FALSE;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint logLen = 0;
		shaderlog = "";
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = new char[logLen];
			GLsizei written;
			glGetProgramInfoLog(handle, logLen, &written, log);
			shaderlog = log;
			delete [] log;
		}
		return false;
	}

	//	GLint logLen = 0;
	//	shaderlog = "";
	//	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
	//	if(logLen > 0)
	//	{
	//		char *log = new char[logLen];
	//		GLsizei written;
	//		glGetProgramInfoLog(handle, logLen, &written, log);
	//		shaderlog = log;
	//		delete [] log;
	//	}
	//	return false;

	linkStatus = true;
	return linkStatus;
}

bool GLSLProgram::use()
{
	if( !linkStatus ) return false;

	glUseProgram(handle);

	return true;
}

const std::string& GLSLProgram::getLog()
{
	return shaderlog;
}

GLuint GLSLProgram::getHandle()
{
	return handle;
}

bool GLSLProgram::isLinked()
{
	return linkStatus;
}

void GLSLProgram::bindAttribLocation(GLuint location, const char *name)
{
	glBindAttribLocation(handle, location, name);
}

void GLSLProgram::bindFragDataLocation(GLuint location, const char *name)
{
	glBindFragDataLocation(handle, location, name);
}

void GLSLProgram::setUniform(const char *name, const glm::vec2 &v)
{
	glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(v));
}

void GLSLProgram::setUniform(const char *name, const glm::vec3 &v)
{
	glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(v));
}

void GLSLProgram::setUniform(const char *name, const glm::vec4 &v)
{
	glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(v));
}

void GLSLProgram::setUniform(const char *name, const glm::mat4 &m)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

void GLSLProgram::setUniform(const char *name, const glm::mat3 &m)
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

void GLSLProgram::setUniform(const char *name, int i)
{
	glUniform1i(getUniformLocation(name), i);
}

void GLSLProgram::setUniform(const char *name, float f)
{
	glUniform1f(getUniformLocation(name), f);
}

void GLSLProgram::setUniform(const char *name, bool b)
{
	glUniform1i(getUniformLocation(name), b);
}

void GLSLProgram::printActiveUniforms()
{
	GLint maxLength, nUniforms;
	glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &nUniforms);
	glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH , &maxLength);

	GLchar * attributeName = (GLchar *) new char[maxLength];

	GLint size, location;
	GLsizei written;
	GLenum type;

	for(int i=0; i < nUniforms; i++)
	{
		glGetActiveUniform(handle, i, maxLength, &written, &size, &type, attributeName);
		location = glGetUniformLocation(handle, attributeName);
		std::cout<< location << " - " << attributeName << "\n"; 
	}
	delete [] attributeName;
}

void GLSLProgram::printActiveAttributes()
{
	GLint maxLength, nAttributes;
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nAttributes);
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH , &maxLength);

	GLchar * attributeName = (GLchar *) new char[maxLength];

	GLint written, size, location;
	GLenum type;

	for(int i=0; i < nAttributes; i++)
	{
		glGetActiveAttrib(handle, i, maxLength, &written, &size, &type, attributeName);
		location = glGetAttribLocation(handle, attributeName);
		std::cout<< location << " - " << attributeName << "\n";
	}
	delete [] attributeName;
}
