#include "texture2D.h"

void Texture2D::bindTexture() const
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

bool Texture2D::loadArrayF(int dimX, int dimY, float *data)
{
	//TODO: Add some checks
	if(sizeof(data) == 0) return false;

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,dimX,dimY,0,GL_RGB,GL_FLOAT,data);
	glBindTexture(GL_TEXTURE_2D,0);

	return true;
}

bool Texture2D::loadArrayC(int dimX, int dimY, char *data)
{
	//TODO: Add some checks
	if(sizeof(data) == 0) return false;

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,dimX,dimY,0,GL_RGB,GL_UNSIGNED_BYTE,data);
	glBindTexture(GL_TEXTURE_2D,0);

	return true;
}

void Texture2D::texParameteri(GLenum param_1, GLenum param_2)
{
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, param_1, param_2);
	glBindTexture(GL_TEXTURE_2D,0);
}
