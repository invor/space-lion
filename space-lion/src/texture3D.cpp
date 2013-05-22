#include "texture3D.h"

void texture3D::bindTexture() const
{
	glBindTexture(GL_TEXTURE_3D, handle);
}

bool texture3D::loadTextureFile(std::string inputPath, glm::ivec3 resolution)
{
	//TODO: Add some checks

	FILE *pFile;

	int size = resolution.x * resolution.y * resolution.z;

	/* Set texture identifier to correct value (namely the filename) */
	filename.assign(inputPath);

	pFile = fopen (inputPath.c_str(), "rb");
	if (pFile==NULL) return false;

	GLfloat *volumeData = NULL;
	volumeData = new GLfloat[size];
	if (volumeData == NULL) {return false;}

	fread(volumeData,sizeof(GLfloat),size,pFile);
	
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_RED,resolution.x,resolution.y,resolution.z,0,GL_RED,GL_FLOAT,volumeData);
	delete [] volumeData;

	return true;
}

bool texture3D::loadArrayF(int dimX, int dimY, int dimZ, float* data)
{
	//TODO: Add some checks
	if(sizeof(data) == 0) return false;

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_RGB,dimX,dimY,dimZ,0,GL_RGB,GL_FLOAT,data);
	glBindTexture(GL_TEXTURE_3D,0);

	return true;
}

void texture3D::texParameteri(GLenum param_1, GLenum param_2)
{
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, param_1, param_2);
	glBindTexture(GL_TEXTURE_3D,0);
}
