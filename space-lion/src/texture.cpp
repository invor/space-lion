#include "texture.h"


texture::texture(void) : type(GL_TEXTURE_2D)
{
}

texture::texture(const char *fn, GLenum dimensionality) : filename(fn), type(dimensionality)
{
}


texture::~texture(void)
{
}

void texture::bindTexture() const
{
	glBindTexture(type, handle);
}

bool texture::loadTexture2D(const char* path)
{
	//TODO: Add some checks
	if(!(type == GL_TEXTURE_2D)) return false;

	glGenTextures(1, &handle);
	glBindTexture(type, handle);

	if(!glfwLoadTexture2D(path,0))return false;

	glGenerateMipmap(type);

	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindTexture(type,0);

	return true;
}

bool texture::loadTexture2D(int dimX, int dimY, float *data)
{
	//TODO: Add some checks

	glGenTextures(1, &handle);
	glBindTexture(type, handle);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(type,0,GL_RGB,dimX,dimY,0,GL_RGB,GL_FLOAT,data);
	glBindTexture(type,0);

	return true;
}

bool texture::loadTexture3D(int dimX, int dimY, int dimZ, const char* path)
{
	//TODO: Add some checks

	FILE *pFile;

	int size = dimX * dimY * dimZ;

	pFile = fopen (path, "rb");
	if (pFile==NULL) return false;

	GLfloat *volumeData = NULL;
	volumeData = new GLfloat[size];
	if (volumeData == NULL) {return false;}

	fread(volumeData,sizeof(GLfloat),size,pFile);
	
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,dimX,dimY,dimZ,0,GL_LUMINANCE,GL_FLOAT,volumeData);
	delete [] volumeData;

	return true;
}

void texture::texParameteri(GLenum param_1, GLenum param_2)
{
	glBindTexture(type, handle);
	glTexParameteri(type, param_1, param_2);
	glBindTexture(type,0);
}
