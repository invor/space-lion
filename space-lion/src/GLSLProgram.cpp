#include "GLSLProgram.h"

GLSLProgram::GLSLProgram()
{
	handle = glCreateProgram();
	linkStatus = false;
}

GLSLProgram::~GLSLProgram()
{
}


char* GLSLProgram::readShaderFile(const char *path)
{
	FILE* in = fopen(path, "rb");
	if (in == NULL) return NULL;
	
	int res_size = BUFSIZ;
	char* res = (char*)malloc(res_size);
	int nb_read_total = 0;
	
	while (!feof(in) && !ferror(in)) {
	  if (nb_read_total + BUFSIZ > res_size) {
	    if (res_size > 10*1024*1024) break;
	    res_size = res_size * 2;
	    res = (char*)realloc(res, res_size);
	  }
	  char* p_res = res + nb_read_total;
	  nb_read_total += fread(p_res, 1, BUFSIZ, in);
	}
	
	fclose(in);
	res = (char*)realloc(res, nb_read_total + 1);
	res[nb_read_total] = '\0';
	return res;
}

GLuint GLSLProgram::getUniformLocation(const char *name)
{
	return glGetUniformLocation(handle, name);
}

bool GLSLProgram::initShaders(const shaderType inType)
{
	type = inType;

	switch(type)
	{
	case PHONG : {
		if(!compileShaderFromFile("../resources/shaders/v_phong.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_phong.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vNormal");
		bindAttribLocation(2,"vTangent");
		bindAttribLocation(3,"vColour");
		bindAttribLocation(4,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	case FLAT : {
		if(!compileShaderFromFile("../resources/shaders/v_flat.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_flat.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vNormal");
		bindAttribLocation(2,"vTangent");
		bindAttribLocation(3,"vColour");
		bindAttribLocation(4,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	case POISSON : {
		if(!compileShaderFromFile("../resources/shaders/v_poisson.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_poisson.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	case FXAA : {
		if(!compileShaderFromFile("../resources/shaders/v_fxaa.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_fxaa.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	case IDLE : {
		if(!compileShaderFromFile("../resources/shaders/v_idle.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_idle.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	case STAMP : {
		if(!compileShaderFromFile("../resources/shaders/v_stamp.glsl",GL_VERTEX_SHADER)) return false;
		if(!compileShaderFromFile("../resources/shaders/f_stamp.glsl",GL_FRAGMENT_SHADER)) return false;
		bindAttribLocation(0,"vPosition");
		bindAttribLocation(1,"vUVCoord");
		if(!link()) return false;
		std::cout<<getLog();
		glUseProgram(0);
		return true;
		break; }
	default : {
		return false;
		break; }
	}

	return false;
}

bool GLSLProgram::compileShaderFromFile(const char *path, GLenum shaderType)
{
	//read shader source code
	const GLchar* shaderSource = readShaderFile(path);
	if (shaderSource == NULL)
	{
		std::cout<<"Shader file not found."<<std::endl;
		return false;
	}

	//create shader object
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	free((void*)shaderSource);

	//compile shader
	glCompileShader(shader);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			std::cout<<"Shader info log:\n"
				<<log
				<<"\n";
			free(log);
		}

		glDeleteShader(shader);
		return false;
	}

	GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			std::cout<<"Shader info log:\n"
				<<log
				<<"\n";
			free(log);
		}

	//attach shader to program
	glAttachShader(handle, shader);

	return true;
}

bool GLSLProgram::link()
{
	glLinkProgram(handle);

	GLint status = GL_FALSE;
	glGetProgramiv(handle, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint logLen;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(handle, logLen, &written, log);
			std::cout<<"Shader info log:\n"
				<<log;
			free(log);
		}
		return false;
	}

	GLint logLen;
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
	if(logLen > 0)
	{
		char *log = (char *)malloc(logLen);
		GLsizei written;
		glGetProgramInfoLog(handle, logLen, &written, log);
		std::cout<<"Shader info log:\n"
			<<log;
		free(log);
	}

	linkStatus = true;
	return linkStatus;
}

bool GLSLProgram::use()
{
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
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nUniforms);
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH , &maxLength);

	GLchar * attributeName = (GLchar *) malloc(maxLength);

	GLint size, location;
	GLsizei written;
	GLenum type;

	for(int i=0; i < nUniforms; i++)
	{
		glGetActiveUniform(handle, i, maxLength, &written, &size, &type, attributeName);
		location = glGetUniformLocation(handle, attributeName);
		std::cout<< location << " - " << attributeName << "\n"; 
	}
	free(attributeName);
}

void GLSLProgram::printActiveAttributes()
{
	GLint maxLength, nAttributes;
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nAttributes);
	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH , &maxLength);

	GLchar * attributeName = (GLchar *) malloc(maxLength);

	GLint written, size, location;
	GLenum type;

	for(int i=0; i < nAttributes; i++)
	{
		glGetActiveAttrib(handle, i, maxLength, &written, &size, &type, attributeName);
		location = glGetAttribLocation(handle, attributeName);
		std::cout<< location << " - " << attributeName << "\n";
	}
	free(attributeName);
}
