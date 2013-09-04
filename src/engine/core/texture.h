#ifndef texture_h
#define texture_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Texture
{
protected:
	/*	
	/	Integer ids seemed like a bad idea for loading texture files,
	/	that themselves -unlike custom material files- won't contain such an id.
	*/
	std::string filename;

	GLuint handle;
public:
	Texture() {}
	Texture(std::string in_filename) : filename(in_filename) {}
	~Texture() {glDeleteTextures(1,&handle);}

	virtual void bindTexture() const = 0;
	virtual void texParameteri(GLenum, GLenum) = 0;

	std::string getFilename() {return filename;}
};

#endif
