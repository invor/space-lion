#include "texture3D.h"

void Texture3D::bindTexture() const
{
	glBindTexture(GL_TEXTURE_3D, m_handle);
}

bool Texture3D::loadTextureFile(std::string inputPath, glm::ivec3 resolution)
{
	//TODO: Add some checks

	FILE *pFile;

	int size = resolution.x * resolution.y * resolution.z;

	/* Set texture identifier to correct value (namely the filename) */
	m_filename.assign(inputPath);

	pFile = fopen (inputPath.c_str(), "rb");
	if (pFile==NULL) return false;

	GLfloat *volumeData = NULL;
	volumeData = new GLfloat[size];
	if (volumeData == NULL) {return false;}

	fread(volumeData,sizeof(GLfloat),size,pFile);
	
	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_3D, m_handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_RED,resolution.x,resolution.y,resolution.z,0,GL_RED,GL_FLOAT,volumeData);
	delete [] volumeData;

	return true;
}

bool Texture3D::loadArrayF(float* data, glm::ivec3 resolution, GLenum internalFormat, GLenum format)
{
	//TODO: Add some checks
	if(sizeof(data) == 0) return false;

	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_3D, m_handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,internalFormat,resolution.x,resolution.y,resolution.z,0,format,GL_FLOAT,data);
	glBindTexture(GL_TEXTURE_3D,0);

	return true;
}

void Texture3D::texParameteri(GLenum param_1, GLenum param_2)
{
	glBindTexture(GL_TEXTURE_3D, m_handle);
	glTexParameteri(GL_TEXTURE_3D, param_1, param_2);
	glBindTexture(GL_TEXTURE_3D,0);
}
