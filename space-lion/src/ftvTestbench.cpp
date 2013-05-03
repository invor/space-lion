#include "ftvTestbench.h"

bool ftvTestbench::readPpmHeader(char* filename, long& headerEndPos, int& imgDimX, int& imgDimY)
{
	/*
	/	Start off with reading the header of the ppm file.
	*/

	char buffer[80];
	FILE *file;

	file = fopen(filename,"rb");
	if(file==NULL) return false;

	/*
	/	Read image dimensions from header.
	*/
	fgets(buffer, 300, file);
	sscanf(buffer, "%*c %*d %d %d", &imgDimX, &imgDimY);

	/*
	/	Get the position of the header end.
	*/
	headerEndPos = ftell(file);

	return true;
}

bool ftvTestbench::readPpmData(char* filename, char* imageData, long dataBegin, int imageSize)
{
	FILE *file;

	file = fopen(filename,"rb");
	if(file==NULL) return false;
	fseek(file,dataBegin,SEEK_SET);
	/*
	/	Store the complete image data in a single float array.
	/	Channel-wise storing is unnecessary for the openGL usage.
	*/
	for(int i=0; i < imageSize; i++)
	{
		imageData[i] = getc(file);
	}
	fclose(file);
	
	return true;
}

bool ftvTestbench::loadImageSequence()
{
	long dataBegin;
	int imgDimX;
	int imgDimY;
	if(!readPpmHeader("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",dataBegin,imgDimX,imgDimY)) return false;
	char* imageData = new char[3*imgDimX*imgDimY];
	if(!readPpmData("../resources/textures/fault_tolerant_vis/ftle/ftle_f_100.ppm",imageData,dataBegin,(3*imgDimX*imgDimY))) return false;

	glGenTextures(1, &images_f[0]);
	glBindTexture(GL_TEXTURE_2D, images_f[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,imgDimX,imgDimY,0,GL_RGB,GL_UNSIGNED_BYTE,imageData);
	glBindTexture(GL_TEXTURE_2D,0);

	delete[] imageData;
	
	imageProcessor = new postProcessor(imgDimX, imgDimY);
}

void ftvTestbench::getImage(GLuint& image, int index)
{
	image = images_f[index];
}

void ftvTestbench::getFrameConfigA(framebufferObject* maskFbo, framebufferObject* imgFbo)
{
	imgFbo->bind();
	imageProcessor->imageToFBO(images_f[0]);
}
